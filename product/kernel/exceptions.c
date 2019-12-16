
void exception_handler(int exception_num, int errcode)
{
	primitive_printf("Exception Number:%d, Error Code:%d\n", exception_num, errcode);
	// wait for any key to reboot os
}
