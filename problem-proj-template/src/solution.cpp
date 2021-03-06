#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cctype>
#include <cassert>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <bitset>
#include <iterator>
#include <functional>
#include <numeric>

#include <unordered_set>
#include <unordered_map>
#include <random>

using namespace std;

typedef long long int64;
typedef unsigned long long uint64;
typedef double float64;

template<typename T> T sqr(T x) { return x * x; }
inline uint64 bit(int num) { return 1ull << num; }
#define forn(i, n) for(int i = 0; i < int(n); ++i)
#define forr(i, n) for(int i = int(n) - 1; i >= 0; --i)
#define all(x) (x).begin(), (x).end()

typedef vector<int> vint;
typedef vector<bool> vbool;
typedef vector<int64> vint64;
typedef pair<int, int> pii;
typedef pair<int64, int64> pii64;




int main()
{
#ifndef ONLINE_JUDGE	
	freopen("input.txt", "r", stdin);
#endif

	int n;
	cin >> n;
	cout << n*n;

	return 0;
}

