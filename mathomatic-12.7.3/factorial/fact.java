/*
 * Factorial function in Java.
 * Written by Gordon McKinney (http://gmckinney.info).
 */
public class MathSupport
{
	public static double fact(double x)
	{
		return fact((int) x);
	}

	public static double fact(int x)
	{
		double ret = 0.0;
		
		if (x >= 0 )
		{
			for (ret=1.0; x>1; ret=ret*x--)
				;
		}
		return ret;
	}
}
