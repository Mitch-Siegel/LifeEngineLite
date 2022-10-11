#include "rng.h"
#include "random"

boost::mt19937 rng (time(0));
// boost::mt19937 rng (0);

// on range [min, max)
float randFloat(float min, float max)
{
    exit(123);
    // return (float)rand()/(float)(RAND_MAX/a);
    boost::uniform_real<float> u(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > gen(rng, u);
    return gen();
}

// on range [min, max]
int randInt(int min, int max)
{
    // return (rand() % (max - min + 1)) + min;


    boost::uniform_int<int> u(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > gen(rng, u);
    return gen();
}

// given a percent value, return whether or not the event happens
bool randPercent(int percent)
{
    // return (randInt(0, 100) <= percent);

    boost::uniform_int<int> u(0, 100);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > gen(rng, u);
    return gen() <= percent;
}
