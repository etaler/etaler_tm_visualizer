/*
tm visualization example - see Etaler TM in action in RT in 3D!
Copyright (C) 2019 Lior Assouline

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Etaler/Etaler.hpp>
#include <Etaler/Backends/CPUBackend.hpp>
#include <Etaler/Algorithms/TemporalMemory.hpp>
#include <Etaler/Encoders/Category.hpp>
using namespace et;

#include <iostream>
#include <numeric>
#include <unistd.h>
#include <Visualizer.hpp>

inline std::string to_string(std::vector<size_t> v)
{
	if(v.size() == 0)
		return "None";

	std::string res = "";
	for(size_t i=0;i<v.size();i++)
		res += std::to_string(v[i]) + std::string(i==v.size()-1 ? "" : " ");
	return res;
}


int main(int argc, char **argv)
{
	char the_path[256];

    getcwd(the_path, 255);
    printf("Executable is run from %s - please make sure shaders resources are on that path...\n", the_path);
	//auto backend = std::make_shared<et::OpenCLBackend>();
	//et::setDefaultBackend(backend.get());
	size_t num_category = 3;
	size_t bits_per_category = 5;
	intmax_t cells_per_column = 2;
	
	intmax_t sdr_size = bits_per_category*num_category;
	TemporalMemory tm({(intmax_t)sdr_size}, cells_per_column);

	Tensor last_state = zeros({sdr_size, cells_per_column}, DType::Bool);
	Tensor last_pred = zeros({sdr_size, cells_per_column}, DType::Bool);

	std::unique_ptr<Visualizer> vis = std::make_unique<Visualizer>(cells_per_column, bits_per_category*num_category);

	for(size_t i=0;i<40;i++) {
		size_t categoery = i%num_category;
		Tensor x = encoder::category(categoery, num_category, bits_per_category);

		auto [pred, active] = tm.compute(x, last_pred);

		auto active_buff = active.toHost<uint8_t>();
		vis->UpdateLayer(0, (bool*)active_buff.data());

		//std::cout << active << std::endl;
		usleep(1000000);

		tm.learn(active, last_state); //Let the TM learn
		last_state = active;
		last_pred = pred;

		//Display results
		auto prediction = sum(pred, 1, DType::Bool);
		std::vector<size_t> pred_category = decoder::category(prediction, num_category);

		std::cout << "input, prediction of next = " << categoery
			<< ", " << to_string(pred_category);
		std::cout << '\n';
	}

}
