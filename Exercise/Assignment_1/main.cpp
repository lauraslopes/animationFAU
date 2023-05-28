#include <iostream>
#include <Eigen/Eigen>

int main(int argc, char ** argv)
{
	// Ass 1.1 - basic vectors
	//
	Eigen::Vector3f v1; //inicializa v1
	v1 << 0,0,0;
	std::cerr << "v1: " << v1 << std::endl;

	v1 << 1,-1,2;
	std::cerr << "v1: " << v1 << std::endl;

	Eigen::Vector3f v2; //inicializa v2
	v2 << 6,5,4.5;
	std::cerr << "v2:\n" << v2 << std::endl;
	
	// simple operations
	Eigen::Vector3f v_sum;
	v_sum << v1 + v2;
	std::cerr << "v_sum: " << v_sum << std::endl;
	
	Eigen::Vector3f v_diff;
	v_diff << v1 - v2;
	std::cerr << "v_diff: " << v_diff << std::endl;
	
	float s_dot;
	s_dot = v1.dot(v2);
	std::cerr << "s_dot: " << s_dot << std::endl; //produto escalar
	
		
	v1 << Eigen::Vector3f(1, 0, 0);
	v2 << Eigen::Vector3f(0, 1, 0);
	Eigen::Vector3f v_cross;
	v_cross << v1.cross(v2);
	std::cerr << "v_cross:\n" << v_cross << std::endl;

	// simple norm
	float len;
	len = v_diff.norm();
	std::cerr << "distance v1 to v2:\n" << len << std::endl;

			
	
	// Ass 1.2 - basic matrix
	//
	Eigen::Matrix3f M1;
	M1 << 0, 0, 0,
		  0, 0, 0,
		  0, 0, 0;
	std::cerr << "M1:\n" << M1 << std::endl;
	
	Eigen::Matrix3f M2;
	M2 << 1, 0, 0,
		  0, 1, 0,
		  0, 0, 1;
	std::cerr << "M2:\n" << M2 << std::endl;

	// simple operations
	Eigen::Matrix3f m_sum;
	m_sum << M1 + M2;
	std::cerr << "m_sum: " << m_sum << std::endl;
	
	Eigen::Matrix3f m_diff;
	m_diff << M1 - M2;
	std::cerr << "m_diff: " << m_diff << std::endl;
	
	Eigen::Matrix3f m_prod;
	m_prod << M1 * M2;
	std::cerr << "m_prod: " << m_prod << std::endl; 

	//the matrix-vector product of M1 and v2
	Eigen::Matrix3f mprodv;
	mprodv << M1 * v2;
	std::cerr << "m_prod: " << mprodv << std::endl; 

	Eigen::Matrix3f m_div;
	m_div << M1 / 10;
	std::cerr << "m_diff: " << m_div << std::endl;
		
	// advanced initialization
	Eigen::Matrix3f M3;  //usar metodo Constant()
	M3 << 2, 2, 2,
		  0, 2, 2,
		  0, 0, 2;
	std::cerr << "M3:\n" << M3 << std::endl;

	// transpose matrix
	M3 << M3.transpose();
	std::cerr << "M3^t:\n" << M3 << std::endl;
	
	/* The transposition is calculated at the same time you write in the destination matrix,
	   that's why when you use the method transpose in the same matrix you'll write on,
	   the result won't be correct, and you'll lose some informations.
	   It was espected to be: 2, 0, 0
	   						  2, 2, 0
	   						  2, 2, 2

	   But resulted in: 2, 2, 2
	   					2, 2, 2
	   					2, 2, 2 

	*/
	
	// resolve
	M3 << 2, 2, 2,
		  0, 2, 2,
		  0, 0, 2;
	Eigen::Matrix3f temp;
	temp << M3.transpose();
	M3 << temp;
	std::cerr << "M3^t:\n" << M3 << std::endl;
	
	// simple example inverse
	Eigen::Matrix3f M4 = Eigen::Matrix3f::Identity();
	Eigen::Matrix3f inverse;
	M4(0,1) = 2.0f;
	M4(1,0) = 0.4f;
	M4(2,0) = 0.6f;
	inverse = M4.inverse();
	std::cerr << "M4^-1:\n" << inverse << std::endl;
	// Invert the matrix M 4 and output the result.
	
	// simple example orthogonal
	Eigen::Matrix3f M5;
	M5 << 0, 1, 0,
		 -1, 0, 0,
		  0, 0, 1; 
	float det;
	det = M5.determinant();
	if (det == 1)
		std::cerr << "It's orthogonal, determinant = " << det << std::endl;

	// Ass 1.4 - block operations
	//
	
	std::cerr << "\n2nd col: " << M5.col(2-1) << std::endl;
	std::cerr << "\n2nd row: " << M5.row(2-1) << std::endl;
	
	
	Eigen::Matrix3f M_rot = M5;
	Eigen::Vector3f v_trans(1.0f, 0.0f, 0.223f);
	Eigen::Vector3f x(0.0f, 0.5f, 0.333f);

	// Ass 1.5 - very advanced matrices and transformation
	// Todo: Ass 1.5 (a)
	
	// finale
	// TODO: Ass 1.5 (b)
	
	return 0;
}
