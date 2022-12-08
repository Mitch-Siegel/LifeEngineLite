#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include "datatracker.h"

template <class T>
DataTracker<T>::DataTracker(int maxSamples)
{
    this->maxSamples = maxSamples;
    this->data = new T[maxSamples * 2];
}

template <class T>
DataTracker<T>::DataTracker(const DataTracker<T>&d)
{
    printf("Copy constructor of DataTracker called - not intended to be used in this way!\n");
    exit(1);
}

template <class T>
void DataTracker<T>::operator=(const DataTracker<T>&d)
{
    printf("= operator of DataTracker called - not intended to be used in this way!\n");
    exit(1);
}

template <class T>
DataTracker<T>::~DataTracker()
{
    delete[] this->data;
}

template <class T>
void DataTracker<T>::Add(T value)
{
    this->data[this->dataP] = value;
    if (++this->dataP >= (maxSamples * 2))
    {
        for (int i = 0; i < maxSamples; i++)
        {
            this->data[i] = this->data[i + this->maxSamples];
        }
        this->dataP = maxSamples;
    }
}

template <class T>
T const *DataTracker<T>::rawData()
{
    int dataStartP = dataP - maxSamples;
    if (dataStartP < 0)
    {
        dataStartP = 0;
    }
    return data + dataStartP;
}

template <class T>
size_t DataTracker<T>::size()
{
    if (dataP > maxSamples)
    {
        return maxSamples;
    }
    else
    {
        return dataP;
    }
}

template class DataTracker<int>;
template class DataTracker<size_t>;
template class DataTracker<float>;
template class DataTracker<double>;
