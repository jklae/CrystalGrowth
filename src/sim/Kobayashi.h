#pragma once
#include <vector>
#include <assert.h>


#include <iostream>
#include <ctime>



class Kobayashi
{
private :

	const double pi = 3.1415926535;

	double _dx;
	double _dy;
	double _dt;
	double tau;
	double epsilonBar;
	double mu;
	double K;
	double delta;
	double anisotropy;
	double alpha;
	double gamma;
	double tEq;

	std::vector<std::vector<double>> _t;
	std::vector<std::vector<double>> _epsilon;
	std::vector<std::vector<double>> _epsilonDeriv;
	std::vector<std::vector<double>> _gradPhiX;
	std::vector<std::vector<double>> _gradPhiY;
	std::vector<std::vector<double>> _lapPhi;
	std::vector<std::vector<double>> _lapT;
	std::vector<std::vector<double>> _angl;

	void initVector2D(std::vector<std::vector<double>>& vec2D);
	void computeGradLap(int start, int end);
	void evolution();
	void printParam(std::vector<std::vector<double>>& vectemp, const char* a, bool exp) const;
	void createNuclei(int transX, int transY);

public :

	int _nx;
	int _ny;
	std::vector<double> _x;
	std::vector<double> _y;
	std::vector<std::vector<double>> _phi;

	Kobayashi(int nx, int ny, double spacing);

	void update();

};
