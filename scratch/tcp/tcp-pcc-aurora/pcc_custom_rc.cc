#include "pcc_custom_rc.h"
#include "monitoring-interval.h"
#include <ns3/simulator.h>
#include <time.h>
#include <random>
#include <cmath>

namespace
{
    const ns3::DataRate MIN_SENDING_RATE = ns3::DataRate("512Kbps");
    const ns3::DataRate MAX_SENDING_RATE = ns3::DataRate("90Mbps");
    double elapsed_time = 0.;
}

namespace ns3
{

double mi_metric_recv_dur(MonitorInterval mi)
{
    return (mi.GetRecvEndTime() - mi.GetRecvStartTime()).GetSeconds();
}

double mi_metric_send_dur(MonitorInterval mi)
{
    return (mi.GetSendEndTime() - mi.GetSendStartTime()).GetSeconds();
}

double mi_metric_recv_rate(MonitorInterval mi)
{
    double dur = mi_metric_recv_dur(mi);
    if (dur > 0.0)
        return 8.0 * (mi.GetBytesAcked() - mi.GetAveragePacketSize()) / dur;
    return 0.0;
}

double mi_metric_avg_latency(MonitorInterval mi)
{
    return mi.GetObsRtt();
}

double mi_metric_send_rate(MonitorInterval mi)
{
    double dur = mi_metric_send_dur(mi);
    if (dur > 0.0)
        return 8.0 * mi.GetBytesSent() / dur;
    return 0.0;
}

double mi_metric_loss_ratio(MonitorInterval mi)
{
    if (mi.GetBytesLost() + mi.GetBytesAcked() > 0)
        return mi.GetBytesLost() / double(mi.GetBytesLost() + mi.GetBytesAcked());
    return 0.0;
}

double mi_metric_latency_increase(MonitorInterval mi)
{
    double dur = mi_metric_recv_dur(mi);
    return mi.GetObsRttInflation() * dur;
}

double mi_metric_ack_latency_inflation(MonitorInterval mi)
{
    return mi.GetObsRttInflation();
}

double mi_metric_sent_latency_inflation(MonitorInterval mi)
{
    double dur = mi_metric_send_dur(mi);
    double latency_increase = mi_metric_latency_increase(mi);
    if (dur > 0.0)
        return latency_increase / dur;
    return 0.0;
}

double mi_metric_conn_min_latency(MonitorInterval mi)
{
    static std::map<int, double> conn_min_latencies {};
    double latency = mi_metric_avg_latency(mi);
    int id = mi.GetId(); 
    if (conn_min_latencies.count(id) > 0)
    {
        auto prev_min = conn_min_latencies[id];
        if (latency == 0.0)
            return prev_min;
        else
        {
            if (latency < prev_min)
            {
                conn_min_latencies[id] = latency;
                return latency;
            }
            else
                return prev_min;
        }
    }
    else
    {
        if (latency > 0.0)
        {
            conn_min_latencies[id] = latency;
            return latency;
        }
        else
            return 0.0;
    }
}
    
double mi_metric_send_ratio(MonitorInterval mi)
{
    double thpt = mi_metric_recv_rate(mi);
    double send_rate = mi_metric_send_rate(mi);
    // std::cout << "thpt: " << thpt << " send_rate: " << send_rate << std::endl;
    if ((thpt > 0.0) && (send_rate < thpt))
        return send_rate / thpt;
    return 1.0;
}

double mi_metric_latency_ratio(MonitorInterval mi)
{
    double min_lat = mi_metric_conn_min_latency(mi);
    double cur_lat = mi_metric_avg_latency(mi);
    if (min_lat > 0.0)
        return cur_lat / min_lat;
    return 1.0;
}

States::States() : m_size(30)
{
    m_queue.resize(m_size);
}

void States::AddState(float num)
{
    m_queue.push_back(num);
    if (m_queue.size() > m_size) {
        m_queue.pop_front();
    }
};

std::vector<float> States::ToVector()
{
    std::vector<float> queue(m_size);
    for (uint32_t i = 0; i < m_size; i++) {
        queue[i] = m_queue.at(i);
    }
    return queue;
}

void States::UpdateReward(float reward)
{
    m_last_reward = reward;
}

PccCustomRateController::PccCustomRateController(double call_freq)
{
    // std::cout << "Starting Custom Rate Controller" << std::endl;
    SetOpenGymInterface(OpenGymInterface::Get());
}

void PccCustomRateController::Reset()
{
    // std::cout << "Starting Reset" << std::endl;
}

void PccCustomRateController::MonitorIntervalFinished(const MonitorInterval& mi)
{
    double sent_latency_inflation = mi_metric_sent_latency_inflation(mi);
    double latency_ratio = mi_metric_latency_ratio(mi);
    double send_ratio = mi_metric_send_ratio(mi);
    // std::cout << "MonitorIntervalFinished: " << sent_latency_inflation << " " 
    //           << latency_ratio << " "
    //           << send_ratio << " " << std::endl;

    m_states.AddState(sent_latency_inflation);
    m_states.AddState(latency_ratio);
    m_states.AddState(send_ratio);

    // https://github.com/PCCproject/PCC-RL/blob/64bea6eeee3a3e558449c35b6496cf596f5ded52/src/gym/network_sim.py#L194
    // They change the reward function proportional to the bandwidth and RTT prop configured in the network sim.
    // Check the link to see many of their reward functions.
    double throughput = mi_metric_recv_rate(mi);
    double latency_s = mi_metric_avg_latency(mi);
    double loss = mi_metric_loss_ratio(mi);
    // Aurora Original Reward Function
    // double reward = (10.0 * throughput / (8 * mi.GetAveragePacketSize())) - 1e3 * latency_s - 2e3 * loss;

    // Scaling Parameters
    double conf_mean_bw = 12 * 1e6;
    double conf_rtt_s = 2 * 0.030 + 0.002;

    // Linear Reward Function
    // double reward = 3*1e3 * (throughput / conf_mean_bw - latency_s / (conf_rtt_s * 1.5) - 6 * loss);
    // double reward = 3*1e3 * (throughput / conf_mean_bw - latency_s / (conf_rtt_s * 2) - 6 * loss);

    // Exponential Reward Function
    // double reward = 3*1e3 * (throughput / conf_mean_bw - exp(latency_s/conf_rtt_s - 2) - exp(loss - 0.05) + 1);

    // Boolean Reward Function
    double reward = throughput / conf_mean_bw;
    double scaled_latency = latency_s/(conf_rtt_s * 2);
    if (scaled_latency >= 1.0)
    {
        reward -= scaled_latency;
    }
    double scaled_loss = loss / 0.05;
    if (scaled_loss >= 1.0)
    {
        reward -= scaled_loss;
    }

    // Safety Net
    if (throughput == 0.) // Avoid send nothing as a good option
    {
        reward = ~(1LL<<52);
    }
    m_states.UpdateReward(reward);
    // std::cout << "reward: " << reward << " "
    //           << "send_rate: " << mi_metric_send_rate(mi) << " "
    //           << "throughput: " << throughput << " "
    //           << "send_ratio: " << send_ratio << " "
    //           << "latency_s: " << latency_s << " "
    //           << "loss: " << loss << " "
    //           << "min latency: " << mi_metric_conn_min_latency(mi) << " "
    //           << std::endl;
    elapsed_time = Simulator::Now().GetSeconds();
}

DataRate PccCustomRateController::GetNextSendingRate(DataRate current_rate, Time cur_time) {
    // std::random_device rd{};
    // std::mt19937 gen{rd()};
    
    // double mean = 0;
    // double std_var = 0.25;
    // std::normal_distribution<> d{mean, std_var};
    // // Burn In
    // // for (int n = 0; n != 1000; ++n)
    // //     d(gen);
    // double ratio = d(gen);
    // std::cout << "ratio: " << ratio << std::endl;

    // std::cout << "Notify()" << std::endl;
    Notify(); // Notify Agent
    DataRate new_sending_rate;
    if (m_alpha_sending_rate >= 0.0)
    {
        new_sending_rate = current_rate * (1.0 + m_alpha_sending_rate);
    }
    else
    {
        new_sending_rate = current_rate * (1.0 / (1.0  - m_alpha_sending_rate));
    }

    // new_sending_rate = current_rate * (1.0 + m_alpha_sending_rate);

    if (new_sending_rate < MIN_SENDING_RATE)
    {
        new_sending_rate = MIN_SENDING_RATE;
    }
    else if (new_sending_rate > MAX_SENDING_RATE)
    {
        new_sending_rate = MAX_SENDING_RATE;
    }
    return new_sending_rate;
    // return DataRate("11.5Mbps");
    // return DataRate("512Kbps");
}

// OpenGym interface
Ptr<OpenGymSpace> PccCustomRateController::GetActionSpace()
{
    uint32_t parameterNum = 1;
    float low = -1e12;
    float high = 1e12;
    // float low =  -1.;
    // float high =  1.;
    std::vector<uint32_t> shape = {parameterNum,};
    std::string dtype = TypeNameGet<float> ();

    Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
    return box;
}

bool PccCustomRateController::GetGameOver()
{
    bool isGameOver = false;
    static float stepCounter = 0.0;
    stepCounter += 1;
    if (stepCounter >= 400) {
        isGameOver = true;
    }
    return isGameOver;
    // return false;
}

float PccCustomRateController::GetReward()
{
    return m_states.GetLastReward();
    // Flow Completion Time
    // static double last_time = 0.;
    // double delta_elapsed_time = elapsed_time - last_time;
    // last_time = elapsed_time;
    // return -delta_elapsed_time;
}

std::string PccCustomRateController::GetExtraInfo()
{
    return "Extra Info";
}
bool PccCustomRateController::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
    Ptr<OpenGymBoxContainer<float> > box = DynamicCast<OpenGymBoxContainer<float> >(action);
    m_alpha_sending_rate = box->GetValue(0) * 0.05; // 0.05 is Delta Scale
    // m_alpha_sending_rate = box->GetValue(0);
    return true;
}

Ptr<OpenGymSpace> PccCustomRateController::GetObservationSpace()
{
    // single_obs_min_vec = sender_obs.get_min_obs_vector(self.features)
    // single_obs_max_vec = sender_obs.get_max_obs_vector(self.features)
    // self.observation_space = spaces.Box(np.tile(single_obs_min_vec, self.history_len),
    //                                 np.tile(single_obs_max_vec, self.history_len),
    //                                 dtype=np.float32)
    uint32_t parameterNum = 30;
    std::vector<float> low = {-1., 1., 0., -1., 1., 0., -1., 1., 0., -1,
                               1, 0., -1., 1., 0., -1., 1., 0., -1., 1.,
                               0., -1., 1., 0., -1., 1., 0., -1., 1., 0.,};
    std::vector<float> high = {10., 10000.,  1000., 10., 10000.,  1000., 10., 10000.,  1000., 10.,
                               10000.,  1000., 10., 10000.,  1000., 10., 10000.,  1000., 10., 10000.,
                               1000., 10., 10000., 1000., 10., 10000.,  1000., 10., 10000.,  1000.,};
    std::vector<uint32_t> shape = {parameterNum,};
    std::string dtype = TypeNameGet<float> ();
    Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
    return box;
}

Ptr<OpenGymDataContainer> PccCustomRateController::GetObservation()
{
    uint32_t parameterNum = 30;
    std::vector<uint32_t> shape = {parameterNum,};
    // std::vector<float> obs = {-0.5, 500., 25., -0.5, 500., 25., -0.5, 500., 25., -500.,
    //                             500., 25., -0.5, 500., 25., -0.5, 500., 25., -0.5, 500.,
    //                             25., -0.5, 500., 25., -0.5, 500., 25., -0.5, 500., 25.,};
    std::vector<float> obs = m_states.ToVector();
    Ptr<OpenGymBoxContainer<float> > box = CreateObject<OpenGymBoxContainer<float>>(shape);
    for (uint32_t i=0; i<parameterNum; i++)
    {
        box->AddValue(obs[i]);
    }
    return box;
}

}
