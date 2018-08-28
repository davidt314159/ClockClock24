import java.util.Scanner;
import java.time.LocalTime;

public class ErrorTest {

	public static void main(String[] args) {
		final int[][] targetPositions = {
				{50, 46, 48, 48, 24, 72}, //0
				{61, 52, 61, 48, 61, 0}, //1
				{26, 46, 50, 72, 24, 78}, //2
				{26, 46, 26, 72, 26, 72}, //3
				{52, 52, 24, 48, 61, 0}, //4
				{50, 78, 24, 46, 26, 72}, //5
				{50, 78, 48, 46, 24, 72}, //6
				{26, 46, 61, 48, 61, 0}, //7
				{50, 46, 24, 72, 24, 72}, //8
				{50, 46, 24, 48, 26, 72} //9
		};
		int[][] error = {
				{0, 0, 0, 0, 0, 0}, //hour tens
				{0, 0, 0, 0, 0, 0}, //hour ones
				{0, 0, 0, 0, 0, 0}, //minute tens
				{0, 0, 0, 0, 0, 0} //minute ones
		};
		int hour = 12;
		int minute = 0;
		
		for (int min = 0; min < 24 * 60; min++) {
			
			int prevHour = hour;
			int prevMinute = minute;
			if (minute == 59){
				minute = 0;
				if (hour == 12){
					hour = 1;
				} else {
					hour++;
				}
			} else {
				minute++;
			}
			
			int[][] errorTemp = error;
			if (hour/10 != prevHour/10){
				errorTemp[0] = new int[6];
			}
			if (hour%10 != prevHour%10){
				errorTemp[1] = new int[6];
			}
			if (minute/10 != prevMinute/10){
				errorTemp[2] = new int[6];
			}
			if (minute%10 != prevMinute%10){
				errorTemp[3] = new int[6];
			}
			
			boolean forward = maxError(errorTemp) <= 0;
			for (int i = 0; i < 6; i++){
				error[0][i] += (getDiff(forward, targetPositions[hour/10][i], targetPositions[prevHour/10][i]));
				error[1][i] += (getDiff(forward, targetPositions[hour%10][i], targetPositions[prevHour%10][i]));
				error[2][i] += (getDiff(forward, targetPositions[minute/10][i], targetPositions[prevMinute/10][i]));
				error[3][i] += (getDiff(forward, targetPositions[minute%10][i], targetPositions[prevMinute%10][i]));
			}
			
			System.out.printf("%02d:%02d\n", hour, minute);
			printError(error);
			System.out.println();
		}
	}
	
	public static int getDiff(boolean forward, int current, int prev){
		if (forward){
			return current - prev >= 0 ? current - prev : current - prev + 96;
		} else {
			return current - prev <= 0 ? current - prev : current - prev - 96;
		}
	}
	
	public static int maxError(int[][] error){
		int max = Integer.MIN_VALUE;
		for (int i = 0; i < error.length; i++){
			for (int j = 0; j < error[i].length; j++){
				if (Math.abs(error[i][j]) > Math.abs(max)){
					max = error[i][j];
				}
			}
		}
		return max;
	}
	
	public static void printError(int[][] error){
		String output = "";
		for (int[] arr : error){
			for (int e : arr){
				output = output + e + " ";
			}
			output = output + "\n";
		}
		System.out.print(output);
	}
}
