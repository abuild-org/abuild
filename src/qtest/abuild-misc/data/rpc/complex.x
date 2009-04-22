struct complex_struct {
    double a;
    double b;
};
typedef struct complex_struct complex;
program RPCTEST {
    version RPCTESTVERSION {
	complex int_to_complex(int) = 1;
	int complex_to_int(complex) = 2;
    }=1;
}=14159;
