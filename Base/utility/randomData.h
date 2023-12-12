#ifndef __UTILITY_RANDOM_DATA_H__
#define __UTILITY_RANDOM_DATA_H__

#include <random>
#include <chrono>
#include <algorithm>

namespace utility
{
    class RandomData
    {
    public:
        RandomData() = default;
        ~RandomData() = default;

        int getUniformInt(int min, int max)
        {
            const auto seed = mSeeder.entropy() ? mSeeder()
                                                : std::chrono::steady_clock::now().time_since_epoch().count();
            std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
            std::uniform_int_distribution<int> dist(min, max);
            return dist(eng);
        }

        double getUniformReal(double min, double max)
        {
            const auto seed = mSeeder.entropy() ? mSeeder()
                                                : std::chrono::steady_clock::now().time_since_epoch().count();
            std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
            std::uniform_real_distribution<double> dist(min, max);
            return dist(eng);
        }

        int getBernoulli(double p = 0.5)
        {
            const auto seed = mSeeder.entropy() ? mSeeder()
                                                : std::chrono::steady_clock::now().time_since_epoch().count();
            std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
            std::bernoulli_distribution dist(p);
            return dist(eng);
        }

        int getBinomial(int num, double p = 0.5)
        {
            const auto seed = mSeeder.entropy() ? mSeeder()
                                                : std::chrono::steady_clock::now().time_since_epoch().count();
            std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
            std::binomial_distribution<int> dist(num, p);
            return dist(eng);
        }

        int getPoisson(double mean = 1.0)
        {
            const auto seed = mSeeder.entropy() ? mSeeder()
                                                : std::chrono::steady_clock::now().time_since_epoch().count();
            std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
            std::poisson_distribution<int> dist(mean);
            return dist(eng);
        }

        double getExponential(double lambda = 1.0)
        {
            const auto seed = mSeeder.entropy() ? mSeeder()
                                                : std::chrono::steady_clock::now().time_since_epoch().count();
            std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
            std::exponential_distribution<double> dist(lambda);
            return dist(eng);
        }

        double getNormal(double mean, double stddev)
        {
            const auto seed = mSeeder.entropy() ? mSeeder()
                                                : std::chrono::steady_clock::now().time_since_epoch().count();
            std::mt19937 eng(static_cast<std::mt19937::result_type>(seed));
            std::normal_distribution<double> dist(mean, stddev);
            return dist(eng);
        }

        std::string getRandomStr(size_t len)
        {
            std::string res(len, 0);
            auto randC = [this]() -> char
            {
                size_t index = getUniformInt(0, sizeof(CHAR) - 1);
                return CHAR[index];
            };

            std::generate_n(res.begin(), len, randC);
            return res;
        }

    private:
        std::random_device mSeeder;
        const char CHAR[63] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    };
}

#endif