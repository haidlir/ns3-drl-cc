# Modified quic-go to support congestion control based on deep reinforcement learning

This repository stores the source code of this paper.
> Naqvi, H. A., Hilman, M. H., & Anggorojati, B. (2023). Implementability improvement of deep reinforcement learning based congestion control in cellular network. In Computer Networks (Vol. 233, p. 109874). Elsevier BV. https://doi.org/10.1016/j.comnet.2023.109874

It is the training part of congestion control based on deep reinforcement learning. The congestion control basically follows Aurora design implemented in the modified ns3's TCP. If you want to deploy the model by following the paper's procedure using QUIC as transport protocol, please go to [this repository](https://github.com/haidlir/quic-go-mod-drl-cc). Besides, you can also deploy the trained mode to the original Aurora which uses UDT as transport protocol. 

> **Note**
> You can find the original ns3 README text in [here](README-original.md).

# How to use
## Dependency
The training process requires python's tensorflow and stable baselines. You need to setup python 3.7.16 and install the package listed in the [requirements file](requirements.txt).
```python
$ pip install -r requirements.txt
```

## Start Training Process
1. Go to the [tcp-pcc-aurora directory](scratch/tcp/tcp-pcc-aurora).
```bash
$ cd ./scratch/tcp/tcp-pcc-aurora
```
2. Run the ns3_stable_solve.py file
```bash
$ python ns3_stable_solve.py --sb-model-name "<saved file name for the trained model>" --tf-model-name "<saved file name for the trained model>" --test --iterations=<number of iterations>
```

# How to cite
```latex
@article{comnet2023-drlcc-quic,
    title = {Implementability improvement of deep reinforcement learning based congestion control in cellular network},
    journal = {Computer Networks},
    volume = {233},
    pages = {109874},
    year = {2023},
    issn = {1389-1286},
    doi = {https://doi.org/10.1016/j.comnet.2023.109874},
    url = {https://www.sciencedirect.com/science/article/pii/S1389128623003195},
    author = {Haidlir Achmad Naqvi and Muhammad Hafizhuddin Hilman and Bayu Anggorojati}
}
```