int isupper(int c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 0xC0 && c <= 0xDD);
}
