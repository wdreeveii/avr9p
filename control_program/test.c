char globalvar = 5;
int init()
{
	static char c;
	c = 2;
	return 4;
}
int run()
{
	int a;
	a = 1;
	return 5;
}
