#include "Kobayashi.h"

using namespace std;

Kobayashi::Kobayashi(int nx, int ny, double spacing) :
	_nx(nx),
	_ny(ny)
{
	//
	_dx = 0.03;
	_dy = 0.03;
	_dt = timestep;
	tau = 0.0003;
	epsilonBar = 0.01;		// mean of epsilon. scaling factor that determines how much the microscopic front is magnified
	mu = 1.0;
	K = 1.8;				// latent heat 
	delta = 0.01;			// strength of anisotropy (speed of growth in preferred directions)
	anisotropy = 6.0;		// degree of anisotropy
	alpha = 0.9;
	gamma = 10.0;
	tEq = 1.0;
	//sdas

	_x.resize(nx);
	_y.resize(ny);

	initVector2D(_phi);
	initVector2D(_t);
	initVector2D(_gradPhiX);
	initVector2D(_gradPhiY);
	initVector2D(_lapPhi);
	initVector2D(_lapT);

	initVector2D(_angl);
	initVector2D(_epsilon);
	initVector2D(_epsilonDeriv);

	// set the position
	for (int i = 0; i < _nx; i++)
	{
		_x[i] = i * spacing - _nx / 2.0 * spacing;
	}

	for (int i = 0; i < _ny; i++)
	{
		_y[i] = i * spacing - _ny / 2.0 * spacing;
	}

	for (int i = 0; i < _nx; i++)
	{
		for (int j = 0; j < _ny; j++)
		{
			_phi[i][j] = 0.0;
		}
	}

	// set the nuclei
	createNuclei(0, 0);

}

void Kobayashi::createNuclei(int transX, int transY)
{
	for (int i = 0; i < _nx; i++)
	{
		for (int j = 0; j < _ny; j++)
		{
			int iIdx = (i - (_nx / 2) + transX);
			int jIdx = (j - (_ny / 2) + transY);

			// circle equation
			if (iIdx * iIdx + jIdx * jIdx < 20)
				_phi[i][j] = 1.0;


		}
	}
}

void Kobayashi::initVector2D(vector<vector<double>>& vec2D)
{
	for (int i = 0; i < _nx; i++)
	{
		vector<double> tmp;
		tmp.resize(_ny);

		vec2D.push_back(tmp);
	}
}

void Kobayashi::computeGradLap(int start, int end)
{
#pragma omp parallel num_threads(24)
	{
#pragma omp for schedule(guided)
		for (int k = 0; k < _nx * _ny; k++)
		{
			int i = k / _nx;
			int j = k % _ny;

			int jp = j + 1;
			int jm = j - 1;
			int ip = i + 1;
			int im = i - 1;

			if (im == -1)
				im = _nx - 1;
			else if (ip == _nx)
				ip = 0;

			if (jm == -1)
				jm = _ny - 1;
			else if (jp == _ny)
				jp = 0;



			_gradPhiX[i][j] = (_phi[ip][j] - _phi[im][j]) / _dx;
			_gradPhiY[i][j] = (_phi[i][jp] - _phi[i][jm]) / _dy;

			_lapPhi[i][j] = (2.0 * (_phi[ip][j] + _phi[im][j] + _phi[i][jp] + _phi[i][jm])
				+ _phi[ip][jp] + _phi[im][jm] + _phi[im][jp] + _phi[ip][jm]
				- 12.0*_phi[i][j])
				/ (3.0*_dx*_dx);
			_lapT[i][j] = (2.0 * (_t[ip][j] + _t[im][j] + _t[i][jp] + _t[i][jm])
				+ _t[ip][jp] + _t[im][jm] + _t[im][jp] + _t[ip][jm]
				- 12.0*_t[i][j])
				/ (3.0*_dx*_dx);


			if (_gradPhiX[i][j] == 0)
				if (_gradPhiY[i][j] < 0)
					_angl[i][j] = -0.5*pi;
				else if (_gradPhiY[i][j] > 0)
					_angl[i][j] = 0.5*pi;

			if (_gradPhiX[i][j] > 0)
				if (_gradPhiY[i][j] < 0)
					_angl[i][j] = 2.0*pi + atan(_gradPhiY[i][j] / _gradPhiX[i][j]);
				else if (_gradPhiY[i][j] > 0)
					_angl[i][j] = atan(_gradPhiY[i][j] / _gradPhiX[i][j]);

			if (_gradPhiX[i][j] < 0)
				_angl[i][j] = pi + atan(_gradPhiY[i][j] / _gradPhiX[i][j]);




			_epsilon[i][j] = epsilonBar * (1.0 + delta * cos(anisotropy*_angl[i][j]));
			_epsilonDeriv[i][j] = -epsilonBar * anisotropy * delta * sin(anisotropy*_angl[i][j]);

		}

	}
	//printParam(_epsilonDeriv, "========_epsilonDeriv===========", true);
}

void Kobayashi::printParam(vector<vector<double>>& vectemp, const char* a, bool exp) const
{
	// column index
	printf("\n%s\n", a);
	for (int j = 0; j < _ny + 1; j++)
	{
		printf(exp ? "%11d  " : "%8d  ", j);
	}
	printf("\n");

	// row 
	for (int i = 0; i < _nx; i++)
	{
		// row index
		printf(exp ? "%11d  " : "%8d  ", i + 1);

		// -- contents
		for (int j = 0; j < _ny; j++)
		{
			printf(exp ? "%11.4e, " : "%8.4lf  ", vectemp[i][j]);
		}
		printf("\n");
		// --

	}
	printf("\n");
}

void Kobayashi::evolution()
{
#pragma omp parallel num_threads(24)
	{
#pragma omp for schedule(guided)
		for (int k = 0; k < _nx * _ny; k++)
		{
			int i = k / _nx;
			int j = k % _ny;

			int jp = j + 1;
			int jm = j - 1;
			int ip = i + 1;
			int im = i - 1;

			if (im == -1)
				im = _nx - 1;
			else if (ip == _nx)
				ip = 0;

			if (jm == -1)
				jm = _ny - 1;
			else if (jp == _ny)
				jp = 0;

			double gradEpsPowX = (_epsilon[ip][j] * _epsilon[ip][j] - _epsilon[im][j] * _epsilon[im][j]) / _dx;
			double gradEpsPowY = (_epsilon[i][jp] * _epsilon[i][jp] - _epsilon[i][jm] * _epsilon[i][jm]) / _dy;

			double term1 = (_epsilon[i][jp] * _epsilonDeriv[i][jp] * _gradPhiX[i][jp]
				- _epsilon[i][jm] * _epsilonDeriv[i][jm] * _gradPhiX[i][jm])
				/ _dy;

			double term2 = -(_epsilon[ip][j] * _epsilonDeriv[ip][j] * _gradPhiY[ip][j]
				- _epsilon[im][j] * _epsilonDeriv[im][j] * _gradPhiY[im][j])
				/ _dx;
			double term3 = gradEpsPowX * _gradPhiX[i][j] + gradEpsPowY * _gradPhiY[i][j];

			double m = alpha / pi * atan(gamma*(tEq - _t[i][j]));

			double oldPhi = _phi[i][j];
			double oldT = _t[i][j];

			_phi[i][j] = _phi[i][j] +
				(term1 + term2 + _epsilon[i][j] * _epsilon[i][j] * _lapPhi[i][j]
					+ term3
					+ oldPhi * (1.0 - oldPhi)*(oldPhi - 0.5 + m))*_dt / tau;
			_t[i][j] = oldT + _lapT[i][j] * _dt + K * (_phi[i][j] - oldPhi);


		}

	}
}

void Kobayashi::update()
{
	clock_t start, finish;
	/*double duration;

	start = clock();*/
	computeGradLap(0, 0);
	evolution();
	/*finish = clock();

	duration = (double)(finish - start) / CLOCKS_PER_SEC;

	cout << duration << "��\n";*/

	/*static int step = 1;
	printf("step %d\n", step);
	step++;*/
}