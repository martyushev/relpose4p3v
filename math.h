#define PI 3.1415926535897932385
#define RATIO 0.6180339887498948482

#include "solvePoly.h"



// uniformly distributed random numbers from [a, b]
inline double rnd(const double &a, const double &b)
{
	return (b-a)*rand()/(double)RAND_MAX+a;
}



// normally distributed random numbers with mathematical expectation=a, stdandard deviation=sigma, Box-Muller method
void rndn(const double a, const double sigma, double x[2])
{
	double s0, s1;
	s0=rnd(1.e-15,1.);
	s1=rnd(1.e-15,2.*PI);
	s0=sigma*sqrt(-2.*log(s0));
	x[0]=s0*cos(s1)+a;
	x[1]=s0*sin(s1)+a;
}



// multiply two 3x3 matrices C=A*B
void mult(const double A[9], const double B[9], double C[9])
{
	for (int i=0; i<3; ++i)
	{
		const int j=3*i;
		for (int k=0; k<3; ++k)
			C[j+k]=A[j]*B[k]+A[j+1]*B[k+3]+A[j+2]*B[k+6];
	}
}



// basis of null space of mxn matrix A using specifically tailored QR factorization
// result is n-m n-vectors Q
// note that matrix A changes during the computation
void nullQR(double A[9][2*NPOINTS], double Q[9][2], const int n, const int m)
{
	const int nm=n-m, m1=m-1;

	// Hauseholder vectors
	for (int j=0; j<m; ++j)
	{
		double t=0;
		for (int i=j+1; i<n; ++i) t+=A[i][j]*A[i][j];
		double mu=sqrt(A[j][j]*A[j][j]+t);
		mu=(A[j][j]<0)? 1./(A[j][j]-mu):1./(A[j][j]+mu);

		for (int i=j+1; i<n; ++i) A[i][j]*=mu;

		double beta=-2./(1.+t*(mu*mu));
		for (int k=j+1; k<m; ++k)
		{
			double w=A[j][k];
			for (int i=j+1; i<n; ++i) w+=A[i][k]*A[i][j];
			w*=beta;
			for (int i=j+1; i<n; ++i) A[i][k]+=A[i][j]*w;
		}
	}

	// multiply m Householder matrices, inverse order is more efficient
	// we only need last n-m columns of the resulting matrix
	for (int k=0; k<m1; ++k) memset(Q[k],0,sizeof(double)*m1);
	
	// start from the mth matrix
	double beta=1.;
	for (int i=m; i<n; ++i) beta+=A[i][m1]*A[i][m1];

	beta=-2./beta;
	for (int k=0; k<nm; ++k)
	{
		Q[m1][k]=A[k+m][m1]*beta;
		for (int i=m; i<n; ++i)
			Q[i][k]=(k==i-m)? 1.+A[i][m1]*Q[m1][k]:A[i][m1]*Q[m1][k];
	}
	
	// multiply by the remaining m-1 matrices
	for (int j=m1-1; j>=0; --j)
	{
		double beta=1.;
		for (int i=j+1; i<n; ++i) beta+=A[i][j]*A[i][j];
		beta=-2./beta;
		for (int k=0; k<nm; ++k)
		{
			double w=Q[j][k];
			for (int i=j+1; i<n; ++i) w+=Q[i][k]*A[i][j];
			w*=beta;
			Q[j][k]+=w;
			for (int i=j+1; i<n; ++i) Q[i][k]+=A[i][j]*w;
		}
	}
}



// compute adjoint to 3x3 matrix A, i.e. det(A) A^{-1}
void adjoint(const double A[9], double B[9], bool sym)
{
	B[0]=A[4]*A[8]-A[5]*A[7];
	B[1]=-A[1]*A[8]+A[2]*A[7];
	B[2]=A[1]*A[5]-A[2]*A[4];
	B[4]=A[0]*A[8]-A[2]*A[6];
	B[5]=-A[0]*A[5]+A[2]*A[3];
	B[8]=A[0]*A[4]-A[1]*A[3];
	if (sym)
	{
		B[3]=B[1];
		B[6]=B[2];
		B[7]=B[5];
		
	}
	else
	{
		B[3]=-A[3]*A[8]+A[5]*A[6];
		B[6]=A[3]*A[7]-A[4]*A[6];
		B[7]=-A[0]*A[7]+A[1]*A[6];
	}
}



// transpose of 3x3 matrix A
void transpose(const double A[9], double B[9])
{
	B[0]=A[0];
	B[1]=A[3];
	B[2]=A[6];
	B[3]=A[1];
	B[4]=A[4];
	B[5]=A[7];
	B[6]=A[2];
	B[7]=A[5];
	B[8]=A[8];
}



inline double getVol(const double A[3][NPOINTS], const int ind[NPOINTS])
{
	const double m1=A[1][ind[0]]*A[2][ind[1]]-A[2][ind[0]]*A[1][ind[1]],
		m2=A[0][ind[1]]*A[2][ind[0]]-A[0][ind[0]]*A[2][ind[1]],
		m3=A[0][ind[0]]*A[1][ind[1]]-A[0][ind[1]]*A[1][ind[0]];
	
	return A[0][ind[2]]*m1+A[1][ind[2]]*m2+A[2][ind[2]]*m3;
}



// permute image points to improve numerics
inline void iniPerm(const double data[NVIEWS][3][NPOINTS], double A[NVIEWS+1][3][NPOINTS])
{
	int m=0, ind[4][NPOINTS]={{2,3,1,0}, {2,0,3,1}, {0,1,3,2}, {2,1,0,3}};
	double maxVol=fabs(getVol(data[0],ind[0]));
	for (int i=1; i<4; ++i)
	{
		const double vol=fabs(getVol(data[0],ind[i]));
		if (vol<maxVol)
		{
			m=i;
			maxVol=vol;
		}
	}

	for (int j=0; j<NVIEWS; ++j)
		for (int i=0; i<NPOINTS; ++i)
			for (int k=0; k<3; ++k)
				A[j][k][i]=data[j][k][ind[m][i]];
}



// triangulate scene point Q_i using the pair of cameras [I 0] and Rt = [R t]
void triang2v(const double A[NVIEWS+1][3][NPOINTS], const double Rt[12], const int &i, double Q[NPOINTS][3])
{
	const double t1=((Rt[3]*A[0][0][i]+Rt[4]*A[0][1][i]+Rt[5]*A[0][2][i])*A[1][2][i]-(Rt[6]*A[0][0][i]+Rt[7]*A[0][1][i]+Rt[8]*A[0][2][i])*A[1][1][i]);
	const double t2=((Rt[0]*A[0][0][i]+Rt[1]*A[0][1][i]+Rt[2]*A[0][2][i])*A[1][2][i]-(Rt[6]*A[0][0][i]+Rt[7]*A[0][1][i]+Rt[8]*A[0][2][i])*A[1][0][i]);
	double Q3;
	if (fabs(t1)>fabs(t2))
	{
		const double t=Rt[11]*A[1][1][i]-Rt[10]*A[1][2][i];
		Q3=t/t1;
	}
	else
	{
		const double t=Rt[11]*A[1][0][i]-Rt[9]*A[1][2][i];
		Q3=t/t2;
	}

	Q[i][0]=A[0][0][i]*Q3;
	Q[i][1]=A[0][1][i]*Q3;
	Q[i][2]=A[0][2][i]*Q3;
}



// check the cheirality constraint
bool cheirality(const double A[NVIEWS+1][3][NPOINTS], double Rt[12], double Q[NPOINTS][3])
{
	triang2v(A,Rt,0,Q); // triangulate first scene point
	
	const double c1=Q[0][2], c2=Rt[6]*Q[0][0]+Rt[7]*Q[0][1]+Rt[8]*Q[0][2]+Rt[11];
	if (c1>0 && c2>0); // Rt2 is correct
	else if (c1<0 && c2<0) // R2 is correct, t2=-t2
	{
		Rt[9]=-Rt[9];
		Rt[10]=-Rt[10];
		Rt[11]=-Rt[11];
	}
	else
	{ // in this case R2 = Ht*R2, where Ht = -I + 2*(t2*t2')/(t2'*t2)
		const double b=2./(Rt[9]*Rt[9]+Rt[10]*Rt[10]+Rt[11]*Rt[11]);
		for (int j=0; j<3; ++j)
		{
			const int j3=j+3, j6=j+6;
			const double w=b*(Rt[9]*Rt[j]+Rt[10]*Rt[j3]+Rt[11]*Rt[j6]);
			Rt[j]=Rt[9]*w-Rt[j];
			Rt[j3]=Rt[10]*w-Rt[j3];
			Rt[j6]=Rt[11]*w-Rt[j6];
		}
		triang2v(A,Rt,0,Q);
		const double c1=Q[0][2], c2=Rt[6]*Q[0][0]+Rt[7]*Q[0][1]+Rt[8]*Q[0][2]+Rt[11];
		if (c1>0 && c2>0); // Rt2 is correct
		else // R2 is correct, t2=-t2
		{
			Rt[9]=-Rt[9];
			Rt[10]=-Rt[10];
			Rt[11]=-Rt[11];
		}
	}
	
	for (int i=0; i<NPOINTS; ++i)
		triang2v(A,Rt,i,Q); // triangulate all scene points
	
	// check that the rest of scene points are in front of the first two cameras
	// if any of them is not, then the solution is dropped
	for (int i=1; i<NPOINTS; ++i)
		if (Q[i][2]<0 || Rt[6]*Q[i][0]+Rt[7]*Q[i][1]+Rt[8]*Q[i][2]+Rt[11]<0) return 0;

	return 1;
}



void getA(double A[NVIEWS][3][NPOINTS])
{
	for (int j=0; j<NVIEWS1; ++j)
		for (int i=0; i<NPOINTS; ++i)
		{
			const double fac=1./A[j][2][i];
			for (int k=0; k<3; ++k) A[j][k][i]*=fac;
		}
	
	for (int i=0; i<3; ++i)
	{
		const double fac=1./sqrt(A[2][0][i]*A[2][0][i]+A[2][1][i]*A[2][1][i]+A[2][2][i]*A[2][2][i]);
		for (int k=0; k<3; ++k) A[2][k][i]*=fac;
	}
	double fac=1./A[2][2][3];
	for (int k=0; k<3; ++k) A[2][k][3]*=fac;
	
	const double m1=A[2][1][0]*A[2][2][1]-A[2][2][0]*A[2][1][1],
		m2=A[2][0][1]*A[2][2][0]-A[2][0][0]*A[2][2][1],
		m3=A[2][0][0]*A[2][1][1]-A[2][0][1]*A[2][1][0];

	const double idet=1./(A[2][0][2]*m1+A[2][1][2]*m2+A[2][2][2]*m3);
	// inverse of matrix
	A[3][0][0]=(A[2][1][1]*A[2][2][2]-A[2][2][1]*A[2][1][2])*idet;
	A[3][0][1]=(A[2][2][0]*A[2][1][2]-A[2][1][0]*A[2][2][2])*idet;
	A[3][0][2]=m1*idet;
	A[3][1][0]=(A[2][0][2]*A[2][2][1]-A[2][0][1]*A[2][2][2])*idet;
	A[3][1][1]=(A[2][0][0]*A[2][2][2]-A[2][0][2]*A[2][2][0])*idet;
	A[3][1][2]=m2*idet;
	A[3][2][0]=(A[2][0][1]*A[2][1][2]-A[2][0][2]*A[2][1][1])*idet;
	A[3][2][1]=(A[2][0][2]*A[2][1][0]-A[2][0][0]*A[2][1][2])*idet;
	A[3][2][2]=m3*idet;

	A[3][0][3]=2.*(A[2][0][1]*A[2][0][2]+A[2][1][1]*A[2][1][2]+A[2][2][1]*A[2][2][2]);
	A[3][1][3]=2.*(A[2][0][0]*A[2][0][2]+A[2][1][0]*A[2][1][2]+A[2][2][0]*A[2][2][2]);
	A[3][2][3]=2.*(A[2][0][0]*A[2][0][1]+A[2][1][0]*A[2][1][1]+A[2][2][0]*A[2][2][1]);
}