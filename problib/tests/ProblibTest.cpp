#include "..\external\testlib\testlib.h"
#include "..\src\problib.h"
#include <iostream>

using namespace problib;

int main(int argc, char* argv[])
{
	using namespace problib;

	args.initRegisterGen(argc, argv, { "n=[15,35]", "m=89", "k={1, [43,65],[2,3],33}" });

	auto arg_n = args["n"];
	auto arg_m = args["m"];
	auto arg_k = args["k"].ranges<int>(); 
	auto arg_p = args["p"];

	int k1 = arg_k.get_rnd();
	int k2 = arg_k.get_rnd();
	int k3 = arg_k.get_rnd(3, 5);

	std::vector<int> v{ 1, 2, 3, 4, 5, 6 };

	std::cerr << make_printer(v.begin(), v.end()) << std::endl;

	return 0;
}

