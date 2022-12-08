#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

template <class T>
class DataTracker
{
private:
	int maxSamples = 0;
	T *data = nullptr;
	int dataP = 0;

public:
	explicit DataTracker(int maxSamples);

	DataTracker(const DataTracker<T> &d);

	void operator=(const DataTracker<T> &d);

	~DataTracker();

	void Add(T value);

	T const *rawData();

	size_t size();
};
