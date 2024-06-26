struct auxArrays_ns
{
	double q[NVIEWS][NPOINTS][3];
	double K[18];
	double Ht[9];
	double B[2][6];
	double V[4][9];
	double G[5][6];

	void trans(const double [NVIEWS][NPOINTS][3]);
	void getBG();
};



void auxArrays_ns::trans(const double q0[NVIEWS][NPOINTS][3])
{
	iniPerm(q0,q);
	getK(q,K);
}



// compute conics B and G
void auxArrays_ns::getBG()
{
	double C[2*NPOINTS][9], S[NPOINTS][6], H[1][9];
	for (int i=0; i<NPOINTS; ++i)
	{
		const int i2=2*i, i3=i2+1;
		C[i2][0]=C[i2][1]=C[i2][2]=0;
		C[i2][3]=-q[0][i][2]*q[1][i][0];
		C[i2][4]=-q[0][i][2]*q[1][i][1];
		C[i2][5]=-q[0][i][2]*q[1][i][2];
		C[i2][6]=q[0][i][1]*q[1][i][0];
		C[i2][7]=q[0][i][1]*q[1][i][1];
		C[i2][8]=q[0][i][1]*q[1][i][2];
		C[i3][3]=C[i3][4]=C[i3][5]=0;
		C[i3][0]=-C[i2][3];
		C[i3][1]=-C[i2][4];
		C[i3][2]=-C[i2][5];
		C[i3][6]=-q[0][i][0]*q[1][i][0];
		C[i3][7]=-q[0][i][0]*q[1][i][1];
		C[i3][8]=-q[0][i][0]*q[1][i][2];

		S[i][0]=q[0][i][0]*q[0][i][0];
		S[i][1]=2.*q[0][i][1]*q[0][i][0];
		S[i][2]=2.*q[0][i][2]*q[0][i][0];
		S[i][3]=q[0][i][1]*q[0][i][1];
		S[i][4]=2.*q[0][i][2]*q[0][i][1];
		S[i][5]=q[0][i][2]*q[0][i][2];
	}
	
	nullQR<2*NPOINTS,9>(C,H);
	transpose(H[0],Ht);

	nullQR<NPOINTS,6>(S,B);
	double B1[2][9];
	for (int j=0; j<2; ++j)
	{
		B1[j][0]=B[j][0];
		B1[j][1]=B[j][1];
		B1[j][2]=B[j][2];
		B1[j][3]=B[j][1];
		B1[j][4]=B[j][3];
		B1[j][5]=B[j][4];
		B1[j][6]=B[j][2];
		B1[j][7]=B[j][4];
		B1[j][8]=B[j][5];
	}

	double HHt[9], D[9];
	mult(H[0],Ht,HHt);
	adjoint(HHt,D,1);

	double BD[2][9], K[2][9], U1[2][9], U2[2][9], L[2][9], Lt[2][9], t1[2], t2[2];
	for (int j=0; j<2; ++j)
	{
		mult(B1[j],D,BD[j]);
		mult(HHt,B1[j],K[j]);
		t1[j]=B1[j][0]+B1[j][4]+B1[j][8];
		t2[j]=K[j][0]+K[j][4]+K[j][8];
		U1[j][0]=2.*B1[j][0]-t1[j];
		U1[j][1]=2.*B1[j][1];
		U1[j][2]=2.*B1[j][2];
		U1[j][3]=U1[j][1];
		U1[j][4]=2.*B1[j][4]-t1[j];
		U1[j][5]=2.*B1[j][5];
		U1[j][6]=U1[j][2];
		U1[j][7]=U1[j][5];
		U1[j][8]=2.*B1[j][8]-t1[j];
		for (int k=0; k<9; ++k) U2[j][k]=2.*K[j][k];
		U2[j][0]-=t2[j];
		U2[j][4]-=t2[j];
		U2[j][8]-=t2[j];
		mult(U1[j],D,L[j]);
		transpose(L[j],Lt[j]);
	}

	const double t3[3]={t2[0]*t2[0], 2.*t2[0]*t2[1], t2[1]*t2[1]};
	const double t4[4]={t1[0]*t3[0], t1[0]*t3[1]+t1[1]*t3[0], t1[0]*t3[2]+t1[1]*t3[1], t1[1]*t3[2]};

	double tradjK[3];
	tradjK[0]=K[0][0]*(K[0][4]+K[0][8])-K[0][1]*K[0][3]-K[0][2]*K[0][6]+K[0][4]*K[0][8]-K[0][5]*K[0][7];
	tradjK[1]=K[0][0]*(K[1][4]+K[1][8])+K[0][4]*(K[1][0]+K[1][8])+K[0][8]*(K[1][0]+K[1][4])-K[0][1]*K[1][3]-K[0][2]*K[1][6]-K[0][3]*K[1][1]-K[0][5]*K[1][7]-K[0][6]*K[1][2]-K[0][7]*K[1][5];
	tradjK[2]=K[1][0]*(K[1][4]+K[1][8])-K[1][1]*K[1][3]-K[1][2]*K[1][6]+K[1][4]*K[1][8]-K[1][5]*K[1][7];

	double m1, m2, m3, detK[4];
	m1=K[0][4]*K[1][8]-K[0][5]*K[1][7]+K[0][8]*K[1][4]-K[0][7]*K[1][5];
	m2=K[0][5]*K[1][6]-K[0][3]*K[1][8]+K[0][6]*K[1][5]-K[0][8]*K[1][3];
	m3=K[0][3]*K[1][7]-K[0][4]*K[1][6]+K[0][7]*K[1][3]-K[0][6]*K[1][4];
	detK[0]=K[0][0]*(K[0][4]*K[0][8]-K[0][5]*K[0][7])+K[0][1]*(K[0][5]*K[0][6]-K[0][3]*K[0][8])+K[0][2]*(K[0][3]*K[0][7]-K[0][4]*K[0][6]);
	detK[1]=K[0][0]*m1+K[0][1]*m2+K[0][2]*m3+K[0][3]*(K[0][7]*K[1][2]-K[0][8]*K[1][1])+K[0][4]*(K[0][8]*K[1][0]-K[0][6]*K[1][2])+K[0][5]*(K[0][6]*K[1][1]-K[0][7]*K[1][0]);
	detK[2]=K[1][0]*m1+K[1][1]*m2+K[1][2]*m3+K[1][3]*(K[1][7]*K[0][2]-K[1][8]*K[0][1])+K[1][4]*(K[1][8]*K[0][0]-K[1][6]*K[0][2])+K[1][5]*(K[1][6]*K[0][1]-K[1][7]*K[0][0]);
	detK[3]=K[1][0]*(K[1][4]*K[1][8]-K[1][5]*K[1][7])+K[1][1]*(K[1][5]*K[1][6]-K[1][3]*K[1][8])+K[1][2]*(K[1][3]*K[1][7]-K[1][4]*K[1][6]);

	double s[5];
	s[0]=16.*detK[0]*t2[0]+t3[0]*t3[0]-4.*t3[0]*tradjK[0];
	s[1]=16.*detK[0]*t2[1]+16.*detK[1]*t2[0]+2.*t3[0]*t3[1]-4.*t3[0]*tradjK[1]-4.*t3[1]*tradjK[0];
	s[2]=16.*detK[1]*t2[1]+16.*detK[2]*t2[0]+2.*t3[0]*t3[2]+t3[1]*t3[1]-4.*(t3[0]*tradjK[2]+t3[1]*tradjK[1]+t3[2]*tradjK[0]);
	s[3]=16.*detK[2]*t2[1]+16.*detK[3]*t2[0]+2.*t3[1]*t3[2]-4.*t3[1]*tradjK[2]-4.*t3[2]*tradjK[1];
	s[4]=16.*detK[3]*t2[1]+t3[2]*t3[2]-4.*t3[2]*tradjK[2];

	double adjU2[3][9];
	adjoint(U2[0],adjU2[0],0);
	adjoint(U2[1],adjU2[2],0);
	adjU2[1][0]=U2[0][4]*U2[1][8]-U2[0][5]*U2[1][7]-U2[0][7]*U2[1][5]+U2[0][8]*U2[1][4];
	adjU2[1][1]=-U2[0][1]*U2[1][8]+U2[0][2]*U2[1][7]+U2[0][7]*U2[1][2]-U2[0][8]*U2[1][1];
	adjU2[1][2]=U2[0][1]*U2[1][5]-U2[0][2]*U2[1][4]-U2[0][4]*U2[1][2]+U2[0][5]*U2[1][1];
	adjU2[1][3]=-U2[0][3]*U2[1][8]+U2[0][5]*U2[1][6]+U2[0][6]*U2[1][5]-U2[0][8]*U2[1][3];
	adjU2[1][4]=U2[0][0]*U2[1][8]-U2[0][2]*U2[1][6]-U2[0][6]*U2[1][2]+U2[0][8]*U2[1][0];
	adjU2[1][5]=-U2[0][0]*U2[1][5]+U2[0][2]*U2[1][3]+U2[0][3]*U2[1][2]-U2[0][5]*U2[1][0];
	adjU2[1][6]=U2[0][3]*U2[1][7]-U2[0][4]*U2[1][6]-U2[0][6]*U2[1][4]+U2[0][7]*U2[1][3];
	adjU2[1][7]=-U2[0][0]*U2[1][7]+U2[0][1]*U2[1][6]+U2[0][6]*U2[1][1]-U2[0][7]*U2[1][0];
	adjU2[1][8]=U2[0][0]*U2[1][4]-U2[0][1]*U2[1][3]-U2[0][3]*U2[1][1]+U2[0][4]*U2[1][0];
	
	mult(adjU2[0],U1[0],V[0]);
	mult(adjU2[2],U1[1],V[3]);

	double V0[9], V1[9], V2[9], V3[9], V4[9], V5[9];
	mult(adjU2[0],U1[1],V0);
	mult(adjU2[1],U1[0],V1);
	mult(adjU2[1],U1[1],V2);
	mult(adjU2[2],U1[0],V3);
	for (int k=0; k<9; ++k)
	{
		V[1][k]=V0[k]+V1[k];
		V[2][k]=V2[k]+V3[k];
	}

	double adjB[3][9];
	adjoint(B1[0],adjB[0],1);
	adjoint(B1[1],adjB[2],1);
	adjB[1][0]=B[0][3]*B[1][5]-2.*B[0][4]*B[1][4]+B[0][5]*B[1][3];
	adjB[1][1]=B[0][2]*B[1][4]-B[0][1]*B[1][5]+B[0][4]*B[1][2]-B[0][5]*B[1][1];
	adjB[1][2]=B[0][1]*B[1][4]-B[0][2]*B[1][3]-B[0][3]*B[1][2]+B[0][4]*B[1][1];
	adjB[1][3]=adjB[1][1];
	adjB[1][4]=B[0][0]*B[1][5]-2.*B[0][2]*B[1][2]+B[0][5]*B[1][0];
	adjB[1][5]=B[0][1]*B[1][2]-B[0][0]*B[1][4]+B[0][2]*B[1][1]-B[0][4]*B[1][0];
	adjB[1][6]=adjB[1][2];
	adjB[1][7]=adjB[1][5];
	adjB[1][8]=B[0][0]*B[1][3]-2.*B[0][1]*B[1][1]+B[0][3]*B[1][0];

	double X[4][9], Y[3][9]; // X = L*adj(B), Y = B*D*B
	mult(L[0],adjB[0],X[0]);
	mult(L[1],adjB[2],X[3]);
	mult(L[1],adjB[0],V0);
	mult(L[0],adjB[1],V1);
	mult(L[1],adjB[1],V2);
	mult(L[0],adjB[2],V3);
	mult(BD[0],B1[0],Y[0]);
	mult(BD[1],B1[1],Y[2]);
	mult(BD[0],B1[1],V4);
	mult(BD[1],B1[0],V5);
	for (int k=0; k<9; ++k)
	{
		X[1][k]=V0[k]+V1[k];
		X[2][k]=V2[k]+V3[k];
		Y[1][k]=V4[k]+V5[k];
	}

	double Z[2][9]; // Z = B*D+D'*B'
	for (int j=0; j<2; ++j)
	{
		Z[j][0]=2.*BD[j][0];
		Z[j][1]=BD[j][1]+BD[j][3];
		Z[j][2]=BD[j][2]+BD[j][6];
		Z[j][3]=Z[j][1];
		Z[j][4]=2.*BD[j][4];
		Z[j][5]=BD[j][5]+BD[j][7];
		Z[j][6]=Z[j][2];
		Z[j][7]=Z[j][5];
		Z[j][8]=2.*BD[j][8];
	}

	double G1[5][9], G2[5][9], G3[5][9], G4[5][9]; // G1 = L*adj(B)*L', G2 = t3*B*D*B, G3 = t4*(B*D+D'*B'), G4 = t1*t4*D
	mult(X[0],Lt[0],G1[0]);
	mult(X[3],Lt[1],G1[4]);
	mult(X[1],Lt[0],V0);
	mult(X[0],Lt[1],V1);
	mult(X[1],Lt[1],V2);
	mult(X[2],Lt[0],V3);
	mult(X[2],Lt[1],V4);
	mult(X[3],Lt[0],V5);
	for (int k=0; k<9; ++k)
	{
		G1[1][k]=V0[k]+V1[k];
		G1[2][k]=V2[k]+V3[k];
		G1[3][k]=V4[k]+V5[k];
		G2[0][k]=t3[0]*Y[0][k];
		G2[1][k]=t3[0]*Y[1][k]+t3[1]*Y[0][k];
		G2[2][k]=t3[0]*Y[2][k]+t3[1]*Y[1][k]+t3[2]*Y[0][k];
		G2[3][k]=t3[1]*Y[2][k]+t3[2]*Y[1][k];
		G2[4][k]=t3[2]*Y[2][k];
		G3[0][k]=t4[0]*Z[0][k];
		G3[1][k]=t4[0]*Z[1][k]+t4[1]*Z[0][k];
		G3[2][k]=t4[1]*Z[1][k]+t4[2]*Z[0][k];
		G3[3][k]=t4[2]*Z[1][k]+t4[3]*Z[0][k];
		G3[4][k]=t4[3]*Z[1][k];
		G4[0][k]=t4[0]*t1[0]*D[k];
		G4[1][k]=(t4[0]*t1[1]+t4[1]*t1[0])*D[k];
		G4[2][k]=(t4[1]*t1[1]+t4[2]*t1[0])*D[k];
		G4[3][k]=(t4[2]*t1[1]+t4[3]*t1[0])*D[k];
		G4[4][k]=t4[3]*t1[1]*D[k];
	}

	for (int k=0; k<5; ++k)
	{
		G[k][0]=4.*(G1[k][0]-G2[k][0])+2.*G3[k][0]-G4[k][0]+s[k];
		G[k][1]=4.*(G1[k][1]-G2[k][1])+2.*G3[k][1]-G4[k][1];
		G[k][2]=4.*(G1[k][2]-G2[k][2])+2.*G3[k][2]-G4[k][2];
		G[k][3]=4.*(G1[k][4]-G2[k][4])+2.*G3[k][4]-G4[k][4]+s[k];
		G[k][4]=4.*(G1[k][5]-G2[k][5])+2.*G3[k][5]-G4[k][5];
		G[k][5]=4.*(G1[k][8]-G2[k][8])+2.*G3[k][8]-G4[k][8]+s[k];
	}
}