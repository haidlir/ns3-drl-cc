#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
from ns3gym import ns3env

__author__ = "Piotr Gawlowicz"
__copyright__ = "Copyright (c) 2018, Technische Universität Berlin"
__version__ = "0.1.0"
__email__ = "gawlowicz@tkn.tu-berlin.de"

parser = argparse.ArgumentParser(description='Start simulation script on/off')
parser.add_argument('--start',
                    type=int,
                    default=1,
                    help='Start ns-3 simulation script 0/1, Default: 1')
parser.add_argument('--iterations',
                    type=int,
                    default=1,
                    help='Number of iterations, Default: 1')
args = parser.parse_args()
startSim = bool(args.start)
iterationNum = int(args.iterations)

port = 5555
simTime = 10 # seconds
stepTime = 0.5  # seconds
seed = 12
simArgs = {"--duration": simTime,}
debug = False

env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
# simpler:
#env = ns3env.Ns3Env()
ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)
obs = env.reset()
env.close()
quit()
stepIdx = 0
currIt = 0

try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset()
        acc_reward = 0
        done = False
        info = None
        print("Step: ", stepIdx)
        # print("---obs: ", obs)

        while True:
            stepIdx += 1
            action = [0.0] # Increase pacing rate 6% every interval
            # print("---action: ", action)

            # print(" ", stepIdx, end="")
            obs, reward, done, info = env.step(action)
            # print("---obs, reward, done, info: ", obs, reward, done, info)
            acc_reward += reward

            if done:
                stepIdx = 0
                if currIt + 1 < iterationNum:
                    env.reset()
                break
        print("Total reward: ", acc_reward)

        currIt += 1
        if currIt == iterationNum:
            break


except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")