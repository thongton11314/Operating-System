import java.util.Scanner;

public class test {
    public static void main(String[] args) throws InterruptedException {

        // Prints "Hello, World" to the terminal window.
        System.out.print("Write in your name:");
        Scanner scanf = new Scanner(System.in);  // Create a Scanner object
        for(int i = 3; i > -1; i--) {
            Thread.sleep(1000);
            System.out.print(i);
            System.out.println();
        }
        System.out.println("Hello " + scanf.nextLine());
    }   
}