import java.io.BufferedReader;
import java.io.InputStreamReader;

public class TestSystemIn
{
    public static void main(String[] args) throws Exception
    {
        BufferedReader in = new BufferedReader(
	    new InputStreamReader(System.in));
	System.out.println("input: " + in.readLine());
    }
}
