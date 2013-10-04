#include "UtilDefs.h"
#include "Shower.h"
#include "RecoHit.h"
#include <Eigen/Dense>
using namespace Eigen;


#include <limits.h>
#include <sys/timeb.h>
#include <float.h>
#include "ChamberAnalyzer.h"
double getHighResolutionTime(void)
{
	struct timeb tp;
	ftime(&tp);
	return tp.time+0.001*tp.millitm;
}
void fillRandomSingle(int m, int n, float* a, float min, float max)
{
	int i, j;

	srand(1);

	for (j=0; j<m; j++)
	{
		for (i=0; i<n; i++)
		{
			a[j*n+i] = min + (max-min) * rand()/RAND_MAX;
		}
	}
}
#ifdef USE_CULA
culaStatus benchSgesvd(int n)
{
	cublasHandle_t handle;
	int m = 3;

	char jobu = 'A';
	char jobvt = 'A';

	int lda = m;
	int ldu = m;
	int ldvt = n;
	int ucol = IMIN(m,n);

	int info = 0;
	int lwork = -1;

	float *a_cula = NULL;
	float *a_mkl = NULL;
	float *s_cula = NULL;
	float *s_mkl = NULL;
	float *u_cula = NULL;
	float *u_mkl = NULL;
	float *vt_cula = NULL;
	float *vt_mkl = NULL;
	float *work_mkl = NULL;

	double start_time, end_time;
	double cula_time, mkl_time;
	culaStatus status = culaNoError;
	//cublasStatus_t ier=cublasCreate(&handle);

	a_cula = (float*) malloc(lda*n*sizeof(float));
	a_mkl = (float*) malloc(lda*n*sizeof(float));
	s_cula = (float*) malloc(IMIN(m,n)*sizeof(float));
	s_mkl = (float*) malloc(IMIN(m,n)*sizeof(float));
	u_cula = (float*) malloc(ldu*ucol*sizeof(float));
	u_mkl = (float*) malloc(ldu*ucol*sizeof(float));
	vt_cula = (float*) malloc(ldvt*n*sizeof(float));
	vt_mkl = (float*) malloc(ldvt*n*sizeof(float));
	work_mkl = (float*) malloc(1*sizeof(float));

	if(!a_cula || !a_mkl || !s_cula || !s_mkl || !u_cula || !u_mkl || !vt_cula || !vt_mkl || !work_mkl)
	{
		printf(" Host side allocation error.\n");
		status = culaInsufficientMemory;
		goto endBenchSgesvd;
	}

	fillRandomSingle(lda, n, a_cula, 1.0f, 256.0f);
	memcpy(a_mkl, a_cula, lda*n*sizeof(float));

	// Run CULA version
	//start_time = getHighResolutionTime();
	culaSgesvd(jobu, jobvt, m, n, a_cula, lda, s_cula, u_cula, ldu, vt_cula, ldvt);
	
endBenchSgesvd:
	free(a_cula);
	free(a_mkl);
	free(s_cula);
	free(s_mkl);
	free(u_cula);
	free(u_mkl);
	free(vt_cula);
	free(vt_mkl);
	free(work_mkl);
	return status;
}
#endif





void Shower::transverseProfile(uint32_t plan,uint32_t &nh,double &xb,double &yb, double &l0, double &l1,double* v0,double *v1,double &n9,double &n25)
{
	nh=0;xb=0;yb=0;l0=0;l1=0;
	v0[0]=0;v0[1]=0;v0[0]=0;v0[1]=0;
	std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.find(plan);
	if (ipl==thePlans_.end()) return;
	double wt=0.;


	for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
	{
		double w=1.;
		int ithr= iht->getAmplitude()&0x3;
		if (ithr==2) w=1;
		if (ithr==1) w=1;
		if (ithr==3) w=2.;
		xb+=iht->X()*w;
		yb+=iht->Y()*w;
		wt+=w;
		nh++;
	}
	xb=xb/wt;
	yb=yb/wt;
	if (nh<3) return;
	//DEBUG_PRINT("1\n");
	Matrix<double,Dynamic,2> m(nh,2);
	//DEBUG_PRINT("2\n");
	Matrix<double,2,Dynamic> mt(2,nh);
	Matrix<double,Dynamic,Dynamic> D(nh,nh);
	for (int i=0;i<nh;i++)
	for (int j=0;j<nh;j++)
	D(i,j)=0.;
	//DEBUG_PRINT("3 %d\n",nh);


	nh=0;
	wt=0.;
	//firstPlan_=99;
	n9=0;
	n25=0;
	for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
	{
		if (abs(iht->X()-xb)<2 && abs(iht->Y()-yb)<2 ) n9=n9+1;
		if (abs(iht->X()-xb)<3 && abs(iht->Y()-yb)<3 ) n25=n25+1;
		m(nh,0)=iht->X()-xb;
		mt(0,nh) =iht->X()-xb;
		m(nh,1)=iht->Y()-yb;
		mt(1,nh) =iht->Y()-yb;
		int ithr= iht->getAmplitude()&0x3;
		double w=1.;
		if (ithr==2) w=1;
		if (ithr==1) w=1;
		if (ithr==3) w=2.;
		//w=1.;
		D(nh,nh)=w;
		wt+=w;
		nh++;
	}
	
	//DEBUG_PRINT("4\n");
	D *=1./wt;
	//std::cout<<" Here it is "<<std::endl<<D<<std::endl;

	Matrix<double,Dynamic,Dynamic> V(nh,nh);
	// Matrix<double,Dynamic,Dynamic> V1(nh,3);
	//DEBUG_PRINT("5\n");
	//V1= D*m;
	//std::cout<<" Here it is "<<std::endl<<V1<<std::endl;
	//DEBUG_PRINT("=6\n");
	V=mt*D*m;
	//std::cout<<" Here it is "<<std::endl<<V<<std::endl;
	// DEBUG_PRINT("7\n");
	//V *=1./nh;
	//std::cout<<" Here it is "<<std::endl<<V<<std::endl;


	SelfAdjointEigenSolver< Matrix<double,Dynamic,Dynamic> > eigensolver(V);
	if (eigensolver.info() != Success) abort();
	// std::cout << "The eigenvalues of V are:\n" << eigensolver.eigenvalues() << std::endl;
	// std::cout << "Here's a matrix whose columns are eigenvectors of A \n"
	// 	    << "corresponding to these eigenvalues:\n"
	// 	    << eigensolver.eigenvectors() << std::endl;

	Matrix<double,2,1> va=eigensolver.eigenvalues();
	l0=va(0);
	l1=va(1);


	Matrix<double,2,2> vv=eigensolver.eigenvectors();
	v0[0]=vv(0,0);
	v0[1]=vv(1,0);
	v1[0]=vv(0,1);
	v1[1]=vv(1,1);



	return;


}
void Shower::PlayMatrix(uint32_t fp,uint32_t lp)
{
	uint32_t nh=0;
	double xb=0,yb=0,zb=0;
	double wt=0.;

	for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
		if (ipl->first<fp || ipl->first>lp) continue;
		for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
		{
			double w=1.;
			int ithr= iht->getAmplitude()&0x3;
			if (ithr==2) w=1;
			if (ithr==1) w=1;
			if (ithr==3) w=2.;
			xb+=iht->X()*w;
			yb+=iht->Y()*w;
			zb+=iht->Z()*w;
			wt+=w;
			nh++;
		}
	}
	if (nh<5) return;
	//DEBUG_PRINT("1\n");
	Matrix<double,Dynamic,3> m(nh,3);
	//DEBUG_PRINT("2\n");
	Matrix<double,3,Dynamic> mt(3,nh);
	Matrix<double,Dynamic,Dynamic> D(nh,nh);
	for (int i=0;i<nh;i++)
	for (int j=0;j<nh;j++)
	D(i,j)=0.;
	//DEBUG_PRINT("3 %d\n",nh);
	xb=xb/wt;
	yb=yb/wt;
	zb=zb/wt;
	xm_[0]=xb;
	xm_[1]=yb;
	xm_[2]=zb;
	nh=0;
	wt=0.;
	//firstPlan_=99;
	for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
		
		//if (ipl->second.size()>=5 && ipl->first<firstPlan_) firstPlan_=ipl->first;
		if (ipl->first<fp || ipl->first>lp) continue;
		for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
		{
			m(nh,0)=iht->X()-xb;
			mt(0,nh) =iht->X()-xb;
			m(nh,1)=iht->Y()-yb;
			mt(1,nh) =iht->Y()-yb;
			m(nh,2)=iht->Z()-zb;
			mt(2,nh) =iht->Z()-zb;
			int ithr= iht->getAmplitude()&0x3;
			double w=1.;
			if (ithr==2) w=1;
			if (ithr==1) w=1;
			if (ithr==3) w=2.;
			//w=1.;
			D(nh,nh)=w;
			wt+=w;
			nh++;
		}
	}
	//DEBUG_PRINT("4\n");
	D *=1./wt;
	//std::cout<<" Here it is "<<std::endl<<D<<std::endl;

	Matrix<double,Dynamic,Dynamic> V(nh,nh);
	// Matrix<double,Dynamic,Dynamic> V1(nh,3);
	//DEBUG_PRINT("5\n");
	//V1= D*m;
	//std::cout<<" Here it is "<<std::endl<<V1<<std::endl;
	//DEBUG_PRINT("=6\n");
	V=mt*D*m;
	//std::cout<<" Here it is "<<std::endl<<V<<std::endl;
	// DEBUG_PRINT("7\n");
	//V *=1./nh;
	//std::cout<<" Here it is "<<std::endl<<V<<std::endl;


	SelfAdjointEigenSolver< Matrix<double,Dynamic,Dynamic> > eigensolver(V);
	if (eigensolver.info() != Success) abort();
	// std::cout << "The eigenvalues of V are:\n" << eigensolver.eigenvalues() << std::endl;
	// std::cout << "Here's a matrix whose columns are eigenvectors of A \n"
	// 	    << "corresponding to these eigenvalues:\n"
	// 	    << eigensolver.eigenvectors() << std::endl;

	Matrix<double,3,1> va=eigensolver.eigenvalues();
	l1_=va(0);
	l2_=va(1);
	l3_=va(2);

	Matrix<double,3,3> vv=eigensolver.eigenvectors();
	v1_[0]=vv(0,0)*sqrt(l1_);
	v1_[1]=vv(1,0)*sqrt(l1_);
	v1_[2]=vv(2,0)*sqrt(l1_);
	v2_[0]=vv(0,1)*sqrt(l2_);
	v2_[1]=vv(1,1)*sqrt(l2_);
	v2_[2]=vv(2,1)*sqrt(l2_);
	v3_[0]=vv(0,2)*sqrt(l3_);
	v3_[1]=vv(1,2)*sqrt(l3_);
	v3_[2]=vv(2,2)*sqrt(l3_);



	//getchar();
	firstPlan_=99;
	lastPlan_=0;
	for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
		if (ipl->second.size()>1 && ipl->first<firstPlan_) firstPlan_=ipl->first;
		lastPlan_=ipl->first;
	}

}
#define IDX2C(i,j,N) (((j)*(N))+(i))
void Shower::culaPrincipalComponents(std::vector<RecoHit*> &v, double result[21])
{
#ifdef USE_CULA
	uint32_t nh=0;
	double xb=0,yb=0,zb=0;
	double wt=0.;

	double fp=DBL_MAX;
	double lp=-DBL_MAX;
	double fx=DBL_MAX;
	double lx=-DBL_MAX;
	double fy=DBL_MAX;
	double ly=-DBL_MAX;

	memset(result,0,21*sizeof(double));
	//INFO_PRINT("%d vector size\n",v.size());
	for (std::vector<RecoHit*>::iterator it=v.begin();it!=v.end();it++)
	{
		RecoHit* iht=(*it);
		if (iht==NULL) continue;
		//INFO_PRINT("%x \n",iht);
		double w=1.;
		int ithr= iht->getAmplitude()&0x3;
		if (ithr==2) w=1;
		if (ithr==1) w=1;
		if (ithr==3) w=2.;
		xb+=iht->X()*w;
		yb+=iht->Y()*w;
		zb+=iht->Z()*w;
		wt+=w;
		if (iht->Z()<fp) fp=iht->Z();
		if (iht->Z()>lp) lp=iht->Z();
		if (iht->I()<fx) fx=iht->I();
		if (iht->I()>lx) lx=iht->I();
		if (iht->J()<fy) fy=iht->J();
		if (iht->J()>ly) ly=iht->J();
		nh++;
	}
	
	if (nh<3) return;
	
	xb=xb/wt;
	yb=yb/wt;
	zb=zb/wt;


	int mc = nh;
	int nc=3;


	int lda = mc;
	int ldu = mc;
	int ldvt = nc;
	int ucol = nc;

	int info = 0;
	int lwork = -1;

	

	
	culaStatus status = culaNoError;
	//cublasStatus_t ier=cublasCreate(&handle);
	//printProblemSize(n);

	float a_cula[lda*nc];
	float s_cula[nc];
	float u_cula[ldu*ucol];
	float vt_cula[ldvt*nc];
	
	//printf("Allocation done %d %d %d \n",mc,nc,lda*nc);

	// store barycenter
	result[0]=xb;
	result[1]=yb;
	result[2]=zb;
	nh=0;
	wt=0.;
	//firstPlan_=99;

	for (std::vector<RecoHit*>::iterator it=v.begin();it!=v.end();it++)
	{
		RecoHit* iht=(*it);	  
		
		//printf("Filling %d row \n",nh);
		a_cula[0*mc+nh]=(iht->X()-xb);
		a_cula[1*mc+nh]=(iht->Y()-yb);
		a_cula[2*mc+nh]=(iht->Z()-zb);
		
		
		wt+=1.;
		nh++;
	}

	//printf("Calling cula \n");
	//float superb[3];
	
	culaSgesvd('S', 'S', mc, nc, a_cula, lda, s_cula, u_cula, ldu, vt_cula, ldvt);
	//LAPACKE_sgesvd(LAPACK_COL_MAJOR,'S', 'S', mc, nc, a_cula, lda, s_cula, u_cula, ldu, vt_cula, ldvt,superb);
	//benchSgesvd(mc,a_cula);
#undef CULA_PRINT
#ifdef CULA_PRINT
	printf("Printing results\n");
	for (int ic=0;ic<nc;ic++)
	printf("%d %f  %f\n",ic,s_cula[ic],s_cula[ic]*s_cula[ic]/wt);
	
	
	printf("vt===>\n");
	for (int ic=0;ic<nc;ic++)
	{	
		for (int jc=0;jc<nc;jc++)
		printf("%f ",vt_cula[ic*nc+jc]);
		printf("\n");
	}
	
	printf("vt===>\n");
	for (int ic=0;ic<nc;ic++)
	{	
		for (int jc=0;jc<nc;jc++)
		printf("%f ",vt_cula[IDX2C(jc,ic,nc)]);
		printf("\n");
	}
#endif	

	result[3]=s_cula[2]*s_cula[2]/wt;
	result[4]=s_cula[1]*s_cula[1]/wt;
	result[5]=s_cula[0]*s_cula[0]/wt;

	// store principal axis
	result[6]=vt_cula[IDX2C(2,0,nc)]*sqrt(result[3]);
	result[7]=vt_cula[IDX2C(2,1,nc)]*sqrt(result[3]);
	result[8]=vt_cula[IDX2C(2,2,nc)]*sqrt(result[3]);
	result[9]=vt_cula[IDX2C(1,0,nc)]*sqrt(result[4]);
	result[10]=vt_cula[IDX2C(1,1,nc)]*sqrt(result[4]);
	result[11]=vt_cula[IDX2C(1,2,nc)]*sqrt(result[4]);
	result[12]=vt_cula[IDX2C(0,0,nc)]*sqrt(result[5]);
	result[13]=vt_cula[IDX2C(0,1,nc)]*sqrt(result[5]);
	result[14]=vt_cula[IDX2C(0,2,nc)]*sqrt(result[5]);

	// Store First and last Z
	result[15]=fp;
	result[16]=lp;
	result[17]=fx;
	result[18]=lx;
	result[19]=fy;
	result[20]=ly;
	
#endif
}



void Shower::computePrincipalComponents(std::vector<RecoHit*> &v, double result[21])
{
	double resultc[21];
	double tbc=getHighResolutionTime();
#ifdef USE_CULA
	if (v.size()>100)
	{

		Shower::culaPrincipalComponents(v, resultc);
		memcpy(result,resultc,21*sizeof(double));
		return;
	}
#endif
	double tac=getHighResolutionTime();
	double tbe=getHighResolutionTime();	

	uint32_t nh=0;
	double xb=0,yb=0,zb=0;
	double wt=0.;

	double fp=DBL_MAX;
	double lp=-DBL_MAX;
	double fx=DBL_MAX;
	double lx=-DBL_MAX;
	double fy=DBL_MAX;
	double ly=-DBL_MAX;

	memset(result,0,21*sizeof(double));
	//INFO_PRINT("%d vector size\n",v.size());
	for (std::vector<RecoHit*>::iterator it=v.begin();it!=v.end();it++)
	{
		RecoHit* iht=(*it);
		if (iht==NULL) continue;
		//INFO_PRINT("%x \n",iht);
		double w=1.;
		int ithr= iht->getAmplitude()&0x3;
		if (ithr==2) w=1;
		if (ithr==1) w=1;
		if (ithr==3) w=2.;
		xb+=iht->X()*w;
		yb+=iht->Y()*w;
		zb+=iht->Z()*w;
		wt+=w;
		if (iht->Z()<fp) fp=iht->Z();
		if (iht->Z()>lp) lp=iht->Z();
		if (iht->I()<fx) fx=iht->I();
		if (iht->I()>lx) lx=iht->I();
		if (iht->J()<fy) fy=iht->J();
		if (iht->J()>ly) ly=iht->J();
		nh++;
	}
	
	if (nh<3) return;
	//INFO_PRINT("%d hits\n",nh);
	Matrix<double,Dynamic,3> m(nh,3);
	//DEBUG_PRINT("2\n");
	Matrix<double,3,Dynamic> mt(3,nh);
	Matrix<double,Dynamic,Dynamic> D(nh,nh);
	for (int i=0;i<nh;i++)
	for (int j=0;j<nh;j++)
	D(i,j)=0.;
	//DEBUG_PRINT("3 %d\n",nh);
	xb=xb/wt;
	yb=yb/wt;
	zb=zb/wt;
#ifdef  USE_CULA_OLD

	int mc = nh;
	int nc=3;
	char jobu = 'A';
	char jobvt = 'A';

	int lda = mc;
	int ldu = mc;
	int ldvt = nc;
	int ucol = nc;

	int info = 0;
	int lwork = -1;

	float *a_cula = NULL;
	float *s_cula = NULL;
	float *u_cula = NULL;
	float *vt_cula = NULL;

	
	culaStatus status = culaNoError;
	//cublasStatus_t ier=cublasCreate(&handle);
	//printProblemSize(n);

	a_cula = new float[lda*nc];
	s_cula = new float[nc];
	u_cula = new float[ldu*ucol];
	vt_cula = new float[ldvt*nc];
	if(!a_cula || !s_cula || !u_cula  || !vt_cula )
	{
		printf(" Host side allocation error.\n");
		status = culaInsufficientMemory;
		exit(-1);
	}
	printf("Allocation done %d %d %d \n",mc,nc,lda*nc);
#endif
	// store barycenter
	result[0]=xb;
	result[1]=yb;
	result[2]=zb;
	nh=0;
	wt=0.;
	//firstPlan_=99;

	for (std::vector<RecoHit*>::iterator it=v.begin();it!=v.end();it++)
	{
		RecoHit* iht=(*it);	  
		m(nh,0)=iht->X()-xb;
		mt(0,nh) =iht->X()-xb;
		m(nh,1)=iht->Y()-yb;
		mt(1,nh) =iht->Y()-yb;
		m(nh,2)=iht->Z()-zb;
		mt(2,nh) =iht->Z()-zb;
		#ifdef USE_CULA_OLD
		printf("Filling %d row \n",nh);
		a_cula[0*mc+nh]=(iht->X()-xb);
		a_cula[1*mc+nh]=(iht->Y()-yb);
		a_cula[2*mc+nh]=(iht->Z()-zb);
		#endif
		int ithr= iht->getAmplitude()&0x3;
		double w=1.;
		if (ithr==2) w=1;
		if (ithr==1) w=1;
		if (ithr==3) w=2.;
		w=1.; //test
		D(nh,nh)=w;
		wt+=w;
		nh++;
	}
#ifdef USE_CULA_OLD
	printf("Calling cula \n");
	double tbc=getHighResolutionTime();
	culaSgesvd('S', 'S', mc, nc, a_cula, lda, s_cula, u_cula, ldu, vt_cula, ldvt);
	double tac=getHighResolutionTime();
	//benchSgesvd(mc,a_cula);
	printf("Printing results\n");
	for (int ic=0;ic<nc;ic++)
	printf("%d %f  %f\n",ic,s_cula[ic],s_cula[ic]*s_cula[ic]/wt);
	
	
	printf("vt===>\n");
	for (int ic=0;ic<nc;ic++)
	{	
		for (int jc=0;jc<nc;jc++)
		printf("%f ",vt_cula[ic*nc+jc]);
		printf("\n");
	}
#endif	
	
	//INFO_PRINT("%d %f\n",nh,wt);
	D *=1./wt;
	//std::cout<<" Here it is "<<std::endl<<D<<std::endl;

	Matrix<double,Dynamic,Dynamic> V(nh,nh);
	// Matrix<double,Dynamic,Dynamic> V1(nh,3);
	//DEBUG_PRINT("5\n");
	//V1= D*m;
	//std::cout<<" Here it is "<<std::endl<<V1<<std::endl;
	//DEBUG_PRINT("=6\n");
	// TEST V=mt*D*m;
	
	V=mt*D*m;
	//std::cout<<" Here it is "<<std::endl<<V<<std::endl;
	// DEBUG_PRINT("7\n");
	//V *=1./nh;
	//std::cout<<" Here it is "<<std::endl<<V<<std::endl;


	SelfAdjointEigenSolver< Matrix<double,Dynamic,Dynamic> > eigensolver(V);
	if (eigensolver.info() != Success) abort();
	
	double tae=getHighResolutionTime();
#ifdef CULA_PRINT

	if (nh>100)
	{
		
		std::cout << "The eigenvalues of V are:\n" << eigensolver.eigenvalues() << std::endl;
		std::cout << "Here's a matrix whose columns are eigenvectors of A \n"
		<< "corresponding to these eigenvalues:\n"
		<< eigensolver.eigenvectors() << std::endl;
		
		printf("%d hits cula=%f / eigen=%f eigen/cula %f \n",nh,tac-tbc,tae-tbe,(tae-tbe)/(tac-tbc));
		
	}
#endif
	//store eigen values
	Matrix<double,3,1> va=eigensolver.eigenvalues();
	result[3]=va(0);
	result[4]=va(1);
	result[5]=va(2);

	// store principal axis
	Matrix<double,3,3> vv=eigensolver.eigenvectors();
	result[6]=vv(0,0)*sqrt(result[3]);
	result[7]=vv(1,0)*sqrt(result[3]);
	result[8]=vv(2,0)*sqrt(result[3]);
	result[9]=vv(0,1)*sqrt(result[4]);
	result[10]=vv(1,1)*sqrt(result[4]);
	result[11]=vv(2,1)*sqrt(result[4]);
	result[12]=vv(0,2)*sqrt(result[5]);
	result[13]=vv(1,2)*sqrt(result[5]);
	result[14]=vv(2,2)*sqrt(result[5]);

	// Store First and last Z
	result[15]=fp;
	result[16]=lp;
	result[17]=fx;
	result[18]=lx;
	result[19]=fy;
	result[20]=ly;
#ifdef CULA_PRINT
	if (nh>100)
	{
		for (uint32_t ic=0;ic<21;ic++)
		printf("%.2f ",resultc[ic]);
		printf("\n");
		for (uint32_t ic=0;ic<21;ic++)
		printf("%.2f ",result[ic]);
		printf("\n");
		getchar();
	}
#endif
#ifdef USE_CULA_OLD

	delete a_cula;
	delete s_cula;
	delete u_cula;
	delete vt_cula;
#endif

}



double Shower::closestDistance(Shower& sh)
{
	//  produit vectoriel des 2 vecteurs directeurs
	double *pv3=sh.getv3();
	double *pxm=sh.getxm();
	double wx= (v3_[1]*pv3[2]-v3_[2]*pv3[1])/sqrt(l3_)/sqrt(sh.getl3());
	double wy= (v3_[2]*pv3[0]-v3_[0]*pv3[2])/sqrt(l3_)/sqrt(sh.getl3());
	double wz= (v3_[0]*pv3[1]-v3_[1]*pv3[0])/sqrt(l3_)/sqrt(sh.getl3());

	if (sqrt(wx*wx+wy*wy+wz*wz)<1E-2) return 99999.;


	DEBUG_PRINT("%f %f %f \n",wx,wy,wz);
	// Plan P1

	double p1=-1*(wx*xm_[0]+wy*xm_[1]+wz*xm_[2]);
	double p2=-1*(wx*pxm[0]+wy*pxm[1]+wz*pxm[2]);

	// Le point (0,-p2/wy,0) appartient a P2  et sa distance a P1 = |a1 x +b1 y + c1 z + p1|/ sqrt(a^2+b^2+c^2)
	double dist = abs(-p2 + p1)/sqrt(wx*wx+wy*wy+wz*wz);
	return dist;

}
Shower::Shower(RecoHit &h) : selected_(false)
{
	h.setShower(this);
	std::vector<RecoHit> v;
	v.push_back(h);
	std::pair<uint32_t,std::vector<RecoHit> > p(h.chamber(),v);
	thePlans_.insert(p);
	firstPlan_=h.chamber();
	lastPlan_=h.chamber();

}
void Shower::clear() {thePlans_.clear();}
bool Shower::append(RecoHit& h,float dist_cut)
{
	bool  append=false;

	for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
		if (abs(h.chamber()-1.*ipl->first)>8) continue;

		for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
		{
			uint32_t iDist=abs(iht->chamber()-h.chamber())+2*(abs(h.I()-iht->I())+abs(h.J()-iht->J()));
			if (iDist<dist_cut)
			{append=true;break;}
			
			//  if (iht->Z()==h.Z())
			// 	   {
			// 	 if (sqrt((iht->X()-h.X())*(iht->X()-h.X())+(iht->Y()-h.Y())*(iht->Y()-h.Y()))<3.)
			// 	   {append=true;break;}
			//   }
			// else
			//   if (sqrt((iht->X()-h.X())*(iht->X()-h.X())+(iht->Y()-h.Y())*(iht->Y()-h.Y())+(iht->Z()-h.Z())*(iht->Z()-h.Z()))<7.)
			// 	 {append=true;break;}
		}
		if (append) break;
	}

	if (!append) return false;
	std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.find(h.chamber());
	h.setShower(this);
	if (ipl!=thePlans_.end())
	ipl->second.push_back(h);
	else
	{
		std::vector<RecoHit> v;
		v.push_back(h);
		std::pair<uint32_t, std::vector<RecoHit> >  p(h.chamber(),v);
		thePlans_.insert(p);
	}

	return true;
}

void Shower::Add(RecoHit& h)
{
	std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.find(h.chamber());
	h.setShower(this);
	if (ipl->second.size()>1 && h.chamber()>lastPlan_) lastPlan_=h.chamber();
	if (ipl!=thePlans_.end())
	{
		ipl->second.push_back(h);
		if (ipl->second.size()>1 &&ipl->first<firstPlan_) firstPlan_=ipl->first;
	}
	else
	{
		std::vector<RecoHit> v;
		v.push_back(h);
		std::pair<uint32_t, std::vector<RecoHit> >  p(h.chamber(),v);
		thePlans_.insert(p);
	}

	return;
}
double Shower::Distance(RecoHit& h)
{
	double dist=9999.;
	for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
		for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
		{
			
			//double d=sqrt((iht->X()-h.X())*(iht->X()-h.X())+(iht->Y()-h.Y())*(iht->Y()-h.Y())+(iht->Z()-h.Z())*(iht->Z()-h.Z()));
			double d=abs(iht->chamber()-h.chamber())+2*(abs(h.I()-iht->I())+abs(h.J()-iht->J()));
			if (d<dist) dist=d;

		}
	}
	return dist;
}


uint32_t Shower::getFDHitsN(uint32_t* v,uint32_t thr)
{
	memset(v,0,8*sizeof(uint32_t));
	uint32_t vscale[8]={1,2,3,4,6,8,12,16};
	for (uint32_t iscale=0;iscale<8;iscale++)
	{
		uint32_t scale=vscale[iscale];
		std::vector<uint32_t> vcell;
		vcell.clear();
		for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
		{
			for (std::vector<RecoHit>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
			{
				int ithr= (*ih).getAmplitude()&0x3;
				if (ithr==2 && thr!=0) continue;
				if (ithr==1 && thr!=1) continue;
				if (ithr==3 && thr!=2) continue;
				uint32_t idx=(*ih).I()-1;
				uint32_t jdx=(*ih).J()-1;
				uint32_t kdx=ipl->first-1;
				if (idx<0) continue;
				if (idx>95) continue;
				if (jdx<0) continue;
				if (jdx>95) continue;
				idx=idx/scale;
				jdx=jdx/scale;
				kdx=kdx/scale;
				uint32_t code=(idx<<24)|(jdx<<16)|kdx;
				if (std::find(vcell.begin(), vcell.end(), code)!=vcell.end()) continue;
				vcell.push_back(code);
			}
		}
		v[iscale]=vcell.size();
	}
	// 10 mm

	return v[0];
}



uint32_t Shower::getFDHits(uint32_t* v,uint32_t thr)
{
	memset(v,0,8*sizeof(uint32_t));

	uint32_t n10=0,n20=0,n30=0,n40=0,n60=0,n80=0,n120=0,n150=0;
	for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
		unsigned char v10[96][96];
		unsigned char v20[48][48];
		unsigned char v30[32][32];
		unsigned char v40[24][24];
		unsigned char v60[16][16];
		unsigned char v80[12][12];
		unsigned char v120[8][8];
		unsigned char v150[6][6];
		memset(v10,0,96*96*sizeof(unsigned char));
		memset(v20,0,48*48*sizeof(unsigned char));
		memset(v30,0,32*32*sizeof(unsigned char));
		memset(v40,0,24*24*sizeof(unsigned char));
		memset(v60,0,16*16*sizeof(unsigned char));
		memset(v80,0,12*12*sizeof(unsigned char));
		memset(v120,0,8*8*sizeof(unsigned char));
		memset(v150,0,6*6*sizeof(unsigned char));

		for (std::vector<RecoHit>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
		{
			int ithr= (*ih).getAmplitude()&0x3;
			if (ithr==2 && thr!=0) continue;
			if (ithr==1 && thr!=1) continue;
			if (ithr==3 && thr!=2) continue;
			uint32_t idx=(*ih).I()-1;
			uint32_t jdx=(*ih).J()-1;
			if (idx<0) continue;
			if (idx>95) continue;
			if (jdx<0) continue;
			if (jdx>95) continue;
			v10[idx][jdx]=1;
			v20[idx/2][jdx/2]=1;
			v30[idx/3][jdx/3]=1;
			v40[idx/4][jdx/4]=1;
			v60[idx/6][jdx/6]=1;
			v80[idx/8][jdx/8]=1;
			v120[idx/12][jdx/12]=1;
			v150[idx/16][jdx/16]=1;
		}
		// 10 mm
		for (uint32_t i=0;i<96;i++)
		for (uint32_t j=0;j<96;j++)
		if (v10[i][j]) n10++;
		// 20 mm
		for (uint32_t i=0;i<48;i++)
		for (uint32_t j=0;j<48;j++)
		if (v20[i][j]) n20++;
		// 30 mm
		for (uint32_t i=0;i<32;i++)
		for (uint32_t j=0;j<32;j++)
		if (v30[i][j]) n30++;
		// 40 mm
		for (uint32_t i=0;i<24;i++)
		for (uint32_t j=0;j<24;j++)
		if (v40[i][j]) n40++;
		// 60 mm
		for (uint32_t i=0;i<16;i++)
		for (uint32_t j=0;j<16;j++)
		if (v60[i][j]) n60++;
		// 80 mm
		for (uint32_t i=0;i<12;i++)
		for (uint32_t j=0;j<12;j++)
		if (v80[i][j]) n80++;

		// 120 mm
		for (uint32_t i=0;i<8;i++)
		for (uint32_t j=0;j<8;j++)
		if (v120[i][j]) n120++;
		// 150 mm
		for (uint32_t i=0;i<6;i++)
		for (uint32_t j=0;j<6;j++)
		if (v150[i][j]) n150++;

	}
	v[0]=n10;
	v[1]=n20;
	v[2]=n30;
	v[3]=n40;
	v[4]=n60;
	v[5]=n80;
	v[6]=n120;
	v[7]=n150;
	return n10;
}




uint32_t Shower::getNumberOfHits(uint32_t plan,uint32_t threshold)
{
	std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.find(plan);
	uint32_t nh[3];
	nh[0]=0;
	nh[1]=0;
	nh[2]=0;
	if (ipl!=thePlans_.end())
	{
		
		for (std::vector<RecoHit>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
		{
			int ithr= (*ih).getAmplitude()&0x3;
			if (ithr==2) nh[0]++;
			if (ithr==1) nh[1]++;
			if (ithr==3) nh[2]++;
		}
	}
	return nh[threshold]; 
}
double Shower::getCorrectedNumberOfHits(uint32_t plan,uint32_t threshold,std::map<uint32_t,double* > correff)
{
	std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.find(plan);
	double nh[3];
	nh[0]=0;
	nh[1]=0;
	nh[2]=0;
	if (ipl!=thePlans_.end())
	{
		std::map<uint32_t,double* >::iterator itcor=correff.find(plan);
		double* c=(itcor->second);
		for (std::vector<RecoHit>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
		{
			int ithr= (*ih).getAmplitude()&0x3;
			uint32_t I=((*ih).I()-1)/16;
			uint32_t J=((*ih).J()-1)/16;
			if (ithr==2) nh[0]+=c[I*8+J];
			if (ithr==1) nh[1]+=c[I*8+J];
			if (ithr==3) nh[2]+=c[I*8+J];
		}
	}
	return nh[threshold]; 
}

uint32_t Shower::getNumberOfHits(uint32_t threshold)
{
	return this->getReduceNumberOfHits(threshold);
}
uint32_t Shower::getReduceNumberOfHits(uint32_t threshold,uint32_t firstp,uint32_t lastp)
{
	uint32_t nh=0;
	for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
		if (ipl->first<firstp) continue;
		if (ipl->first>lastp) continue;
		nh+=getNumberOfHits(ipl->first,threshold);
	}
	return nh;
}

uint32_t Shower::getNumberOfMips(uint32_t plan)
{
	return getNumberOfHits(plan,0)+3*getNumberOfHits(plan,1)+15*getNumberOfHits(plan,2);
}


bool Shower::EdgeDetection()
{
  // Initialisation
  unsigned char bufv[60*32*32];
  array3D<unsigned char> imagev;
  imagev.initialise(bufv,60,32,32); imagev.clear();
  //printf("A\n");
  float bufi[60*32*32];
  array3D<float> image3;
  image3.initialise(bufi,60,32,32); image3.clear();
  //printf("B\n");
  unsigned char bufc[60*32*32];
  array3D<unsigned char> cores;
  cores.initialise(bufc,60,32,32); cores.clear();
  //printf("C\n");
  unsigned char bufe[60*32*32];
  array3D<unsigned char> edges;
  edges.initialise(bufe,60,32,32); edges.clear();
  //printf("D\n");
  unsigned char adjBuf[60*32*32];
  array3D<unsigned char> adj;
  adj.initialise(adjBuf,60,32,32);adj.clear();
  //printf("E\n");
  // Fill table imagev
  for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
      for (std::vector<RecoHit>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
	{
	  imagev.setValue(ih->chamber(),ih->I()/3,ih->J()/3,1);
	}
	  
    }
  //printf("F\n");
  ChamberAnalyzer::sobel_volume(imagev,image3);
  //printf("G\n");


  //uint32_t 
  na3_=0;ne3_=0;nc3_=0;
  float w3=0;

  for (uint32_t k=1;k<imagev.getXSize();k++)
    {
      for (uint32_t i=0;i<imagev.getYSize();i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=0;j<imagev.getZSize();j++)
	    {
	      //if (image2y[k][j]>=-1 ) continue;
	      if (imagev.getValue(k,i,j)==1) {
		na3_++;
		w3+=image3.getValue(k,i,j);
	      }
	      if (image3.getValue(k,i,j)>=-20) // -32 si poids -44
		{
		  if (image3.getValue(k,i,j)>=-2) continue;
		  if (imagev.getValue(k,i,j)!=0) {
		    cores.setValue(k,i,j,1);nc3_++;}
					
		  continue;
		}
	      edges.setValue(k,i,j,1);
	      if (image3.getValue(k,i,j)<-25)
	       edges.setValue(k,i,j,2);
	      ne3_++;
	    }
	}
    }
  //printf("na3 %d %d %d\n",na3_,ne3_,nc3_);
  if (ne3_==0 && nc3_==0) {ne3_=na3_;return false;}
  // Add adjacent hit to core
	
  for (uint32_t k=1;k<imagev.getXSize()-1;k++)
    {
      for (uint32_t i=1;i<imagev.getYSize()-1;i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=1;j<imagev.getZSize()-1;j++)
	    {
	      if (edges.getValue(k,i,j)==0) continue;
				
	      for (uint32_t ks=k-1;ks<=k+1;ks++)
		{
		  for (uint32_t is=i-1;is<=i+1;is++)
		    {
		      for (uint32_t js=j-1;js<=j+1;js++)
			{
			  if (cores.getValue(ks,is,js)>0)
			    {
			      adj.setValue(ks,is,js,1);
			      edges.setValue(k,i,j,0);
			      break;
			    }
			}
		    }
		}
				
	    }
	}
    }
  //printf("I\n");
  // Merge the adjacents
  nc3_=0;ne3_=0;
  for (uint32_t k=1;k<imagev.getXSize()-1;k++)
    {
      for (uint32_t i=1;i<imagev.getYSize()-1;i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=1;j<imagev.getZSize()-1;j++)
	    {
	      if (adj.getValue(k,i,j)>0)
		cores.setValue(k,i,j,1);
	      if (edges.getValue(k,i,j)>0) ne3_++;
	    }
	}
    }
  //printf("J\n");
  //Now loop on core hits


  nc3_=na3_-ne3_;
  
  float ratioEdge3= ne3_*100./na3_;
  DEBUG_PRINT("Ratio of Edge3 / All3  = %f  %d %d %d\n",ratioEdge3,na3_,ne3_,nc3_);
  //Store the evnt information
  //printf("K\n");
  if (ratioEdge3>95) return false;
  //printf("L\n");

  // Now build amas and count hits

  // std::vector<Amas> theAmas_;
  //  uint32_t ne[3],nc_[3];
  memset(ne_,0,3*sizeof(uint32_t));
  memset(nc_,0,3*sizeof(uint32_t));
  uint32_t nall=0;
for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
      for (std::vector<RecoHit>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
	{
	    RecoHit& h = (*ih);
	    int ithr=ih->getAmplitude()&0x3;
	    nall++;
	    if (cores.getValue(ih->chamber(),ih->I()/3,ih->J()/3)!=0)
	      {
		bool appended=false;
		for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
		  {
		    appended=(ia->append(&h,2));
		    if (appended) break;
		  }
		if (!appended)
		  {
		    Amas a(&h);
		    theAmas_.push_back(a);
		  }
		if (ithr==1) nc_[1]++;
		if (ithr==2) nc_[0]++;
		if (ithr==3) nc_[2]++;
	      }
	    else
	      {
		if (ithr==1) ne_[1]++;
		if (ithr==2) ne_[0]++;
		if (ithr==3) ne_[2]++;

	      }
	}
    }	 
  
 double guessEnergy=(nall+nc_[2]*(1.5+1.85E-3*(nall*1.-102.)))*0.052;
 DEBUG_PRINT("E= %f Number of Amas %d : %d Core (%d,%d,%d) Edges (%d,%d,%d) \n",guessEnergy,theAmas_.size(),nall,nc_[0],nc_[1],nc_[2],ne_[0],ne_[1],ne_[2]);


  uint32_t theNall=nc_[0]+nc_[1]+nc_[2]+ne_[0]+ne_[1]+ne_[2];
  bool electron=false;
  bool leak=false;
  bool leakt=false;
  double emax =-DBL_MAX;
  double zmax =-DBL_MAX;
  zlast_ =-DBL_MAX;
  zfirst_=DBL_MAX;
  double zlast_Amas_=134.;

  ng_=0;
  std::sort(theAmas_.rbegin(),theAmas_.rend());
  for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
    {
      ia->compute();
      if (ia->size()<=4) continue;
      if (ia->size()>=10)ng_++;
      for (uint32_t i=0;i<21;i++)
	DEBUG_PRINT("%6.3f ",ia->getComponents(i));
      DEBUG_PRINT(" Size %d\n",ia->size()); 
      if (ia->getComponents(2)>zmax) zmax=ia->getComponents(2);
      if (ia->getComponents(16)>zlast_) zlast_=ia->getComponents(16);
      if (ia->getComponents(15)<zfirst_) zfirst_=ia->getComponents(15);
      if (ia->size()>emax)
	{
	  emax=ia->size();
	  if (ia->getComponents(15)<=2.81 && (ia->size()*1./theNall)>0.2 ) 
	    {
	      electron=true;
	    }


	}
      if (ia->getComponents(16)>=zlast_Amas_ && (ia->size())>4)
	{
	  leak=true;
	}
      if (ia->getComponents(17)<4 && (ia->size())>4)
	{
	  leakt=true;
	}
      if (ia->getComponents(18)>93 && (ia->size())>4)
	{
	  leakt=true;
	}

      if (ia->getComponents(19)<4 && (ia->size())>4)
	{
	  leakt=true;
	}
      if (ia->getComponents(20)>93 && (ia->size())>4)
	{
	  leakt=true;
	}

    }
  nafter_=0;
  uint32_t theTag_=0;
  theTag_|=(ng_<<16);



  std::bitset<61> planes(0);
  for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
      for (std::vector<RecoHit>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
	{
	    RecoHit& h = (*ih);
	    if (h.Z()>zlast_) {
	      if (h.chamber()<61)
		planes.set(h.chamber(),true);
	    }
	}
    }
  for (uint32_t i=0;i<51;i++)
    if (planes[i]!=0) nafter_++;




  DEBUG_PRINT("%d good amas First Plane %f Last Plane %f N hit after last %d\n",ng_,zfirst_,zlast_,nafter_);
  /*

  theZMax_=zmax;
  theZfirst__=zfirst_;
  theZlast__=zlast_;
  thePlanafter__=nafter_;


  electron =electron && (ng_<=3) && (zlast_<50.) && nafter_<10;

  // theAllHit_=(na2<<20)|(na1<<10)|na0;
  // theEdgeHit_=(ne2<<20)|(ne1<<10)|ne0;

  if (leak) INFO_PRINT("==================================================> L E A K A G E <==============================\n");
  if (leakt) INFO_PRINT("==================================================> T R A N S V E R S E  L E A K A G E <==============================\n");
  if (electron) INFO_PRINT("==================================================> E L E C T R O N <============================== %d %f %d \n",ng_,zmax,nafter_);


  theTag_=0;
  if (electron) theTag_=1;
  if (leak) theTag_+=2;
  if (leakt) theTag_+=4;
  */

  return true;
}
