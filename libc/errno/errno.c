int errno;

/*
 * separate function for this in case we use threads later.
 */
void
__set_errno(int val)
{
	errno = val;
}
