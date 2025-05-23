import gym
import numpy as np
from stable_baselines3 import PPO
from stable_baselines3.common.envs import DummyVecEnv
import subprocess
import time

class DroneControlEnv(gym.Env):
    """
    Custom Environment for DroneControl game.
    """

    def __init__(self):
        super(DroneControlEnv, self).__init__()
        
        # Action space: [delay, threshold] adjustments
        # E.g., delay: [-50ms, +50ms], threshold: [-10, +10]
        self.action_space = gym.spaces.Box(low=np.array([-50, -10]), high=np.array([50, 10]), dtype=np.float32)
        
        # Observation space: Extracted metrics (e.g., performance, latency)
        self.observation_space = gym.spaces.Box(low=np.array([0, 0]), high=np.array([100, 100]), dtype=np.float32)

    def step(self, action):
        """
        Perform one step in the environment.
        """
        delay_adjustment, threshold_adjustment = action
        
        # Run the game executable with parameters
        result = subprocess.run(
            ["./DroneControl", str(delay_adjustment), str(threshold_adjustment)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        # Parse output (example assumes game outputs performance and latency as CSV)
        try:
            performance, latency = map(float, result.stdout.decode("utf-8").strip().split(","))
        except ValueError:
            performance, latency = 0, 100  # Default values in case of failure
        
        # Observation: Current state metrics
        observation = np.array([performance, latency], dtype=np.float32)
        
        # Reward: Higher performance, lower latency
        reward = performance - latency
        
        # Determine if the episode is done
        done = performance > 95 or latency < 10  # Example conditions

        # Optional: Add additional information
        info = {"performance": performance, "latency": latency}

        return observation, reward, done, info

    def reset(self):
        """
        Reset the environment to an initial state.
        """
        # Optionally reset the game executable
        subprocess.run(["./DroneControl", "reset"])
        
        # Initial observation (example: random values within observation space)
        observation = np.array([50, 50], dtype=np.float32)
        return observation

    def render(self, mode="human"):
        """
        Render the environment (not implemented for CLI interaction).
        """
        pass

    def close(self):
        """
        Clean up resources.
        """
        subprocess.run(["./DroneControl", "shutdown"])

# Instantiate the environment
env = DummyVecEnv([lambda: DroneControlEnv()])

# Train an RL agent
model = PPO("MlpPolicy", env, verbose=1)
model.learn(total_timesteps=10000)

# Test the trained model
obs = env.reset()
for i in range(100):
    action, _ = model.predict(obs)
    obs, reward, done, info = env.step(action)
    print(f"Step {i}, Obs: {obs}, Reward: {reward}, Info: {info}")
    if done:
        print("Episode finished!")
        break

env.close()
