#include<iostream>
#include<complex>
#include<vector>
#include<cmath>
#include<fstream>

using complex = std::complex<double>;

class Newton final {
public:
	std::pair<double, double> c1, c4;
	int height;
	int width;
	std::vector<std::pair<complex, char>> roots;
	int number_of_iterations;

	complex calculate_polinomial(complex meaning) {
		complex res(1, 0);
		for (auto root_n = 0; root_n != roots.size(); root_n++) {
			res *= (meaning - roots[root_n].first);
		}
		return res;
	}

	complex calculate_derivative(complex meaning) {
		complex derivative = complex(0, 0);
		for (auto i = 0; i < roots.size(); ++i) {
			complex var = complex(1, 0);
			for (auto j = 0; j < roots.size(); ++j) {
				if (i != j) {
					var *= meaning - roots[j].first;
				}
			}
			derivative += var;
		}
		return derivative;
	}
public:
	Newton(std::pair<double, double> c1, std::pair<double, double> c4) :
		c1(c1), c4(c4) {
		height = 100;
		width = 100;
		number_of_iterations = 100;
		get_config();
	}

	~Newton() { }
	Newton(const Newton&) = delete;
	Newton& operator=(const Newton&) = delete;
	Newton(const Newton&&) = delete;
	Newton& operator=(const Newton&&) = delete;

	void get_root(double Re, double Im, char color) {
		roots.push_back(std::make_pair(complex(Re, Im), color));
	}
	void zoom(std::pair<double, double> cor1, std::pair<double, double> cor4) {
		c1 = cor1;
		c4 = cor4;
	}
	void method(std::vector<char>& draw, complex a = complex(1, 0)) {
		double fraction_x = (c4.first - c1.first) / width;
		double fraction_y = (c1.second - c4.second) / height;
		for (auto x = 0; x < width; x++) {
			for (auto y = 0; y < height; y++) {
				complex z = complex(c1.first + fraction_x * (0.5 + x), c4.second + fraction_y * (0.5 + y));
				for (auto idx = 1; idx <= number_of_iterations; ++idx) {
					z = z - a * (calculate_polinomial(z) / calculate_derivative(z));
				}
				draw.push_back(find_closest_root(z).second);
			}
		}
	}

	std::pair<complex, char> find_closest_root(complex meaning) {
		double min = -1;
		complex this_root = 0;
		char color = 0;
		for (auto iter = 0; iter != roots.size(); iter++) {
			if (min > abs(meaning - roots[iter].first) || min < 0) {
				min = abs(meaning - roots[iter].first);
				this_root = roots[iter].first;
				color = roots[iter].second;
			}
		}
		return std::make_pair(this_root, color);
	}

	void move_root(char color, std::pair<double, double> new_r) {
		complex new_root = complex(new_r.first, new_r.second);
		for (auto idx = 0; idx != roots.size(); idx++) {
			if (roots[idx].second == color) {
				roots[idx].first = new_root;
				break;
			}
		}
	}
	void get_config() {
		std::ifstream fin("number of iterations.txt");
		if (fin.is_open()) {
			fin >> number_of_iterations;
		}
		fin.close();
	}
	std::pair<int, int> get_dimensions() {
		return { width, height };
	}

};



void method_test() {
	Newton n(std::make_pair(0.1, 1), std::make_pair(2, -1));
	n.get_root(1, 0, '1');
	n.get_root(-1, 0, '-1');
	std::vector<char> help;
	n.method(help);
	bool flag = false;
	for (auto cnt = 0; cnt != help.size(); ++cnt) {
		if (help[cnt] == '1') {
			continue;
		}
		else {
			flag = true;
			break;
		}
	}
	if (flag)
		std::cout << "Does not converge to the right root!" << std::endl;
	else
		std::cout << "Method do work" << std::endl;
}

void test_calculate_polinomial() {
	Newton n(std::make_pair(0.1, 1), std::make_pair(2, -1));
	n.get_root(1, 0, '1');
	n.get_root(-1, 0, '-1');
	std::cout << n.calculate_polinomial(complex(2, 0)) << std::endl;
}

void test_calculate_derivative() {
	Newton n(std::make_pair(0.1, 1), std::make_pair(2, -1));
	n.get_root(1, 0, '1');
	n.get_root(-1, 0, '-1');
	std::cout << n.calculate_derivative(complex(2, 0)) << std::endl;
}

int main() {
	method_test();
	test_calculate_polinomial();
	test_calculate_derivative();
}