#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>

using namespace std;

void play_music(string songName)
{
	string cmd = "canberra-gtk-play -f music/" + songName;
	system(cmd.c_str());
}

void printASCII(string fileName)
{
	string line = "";
	ifstream inFile;
	inFile.open(fileName);
	if (inFile.is_open())
	{
		while(getline(inFile, line))
		{
			cout << line << endl;
		}
	} else {
		cout << "File failed to load. " << endl;
		cout << "Nothing to display." << endl;
	}
	inFile.close();
}

void menu_header()
{
	string fileName = "dos_art.txt";
	printASCII(fileName);
}


void main_menu()
{
	char char_choice[1];
	int int_choice = 0;
	do
	{
		system("clear");
		menu_header();
		cout << "\n";
		cout << "Dominic's CCC menu selection: \n\n";
		cout << "1. setup target parameters\n";
		cout << "2. setup attack type\n";
		cout << "3. start attack\n";
		cout << "4. Exit\n";
		cin >> char_choice;
		int_choice = atoi(char_choice);

		cout << char_choice;
	} while(1 == 1);
}

int main()
{
	thread t1(play_music, "music.wav");
	main_menu();
	system("pause");
	return 0;
}
