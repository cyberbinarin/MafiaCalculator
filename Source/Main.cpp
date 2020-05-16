#include <fstream>
#include <iostream>
#include "GameState.h"
#include "StateSimulator.h"

namespace {
	
	//void _outputNumberDetailed(Number a_number) 
	//{
	//	std::cout << a_number.divident << '/' << a_number.divisor << " (" << 
	//		static_cast<int>(std::round(static_cast<double>(a_number.divident) / static_cast<double>(a_number.divisor) * 100.0))
	//		<< "%)";
	//}

	void _outputNumberDetailed(long double a_number) 
	{
		std::cout << (std::round(a_number * 10000.0) / 100.0) << "%";
	}

	GameState _CreateGameState(int players, int mafia, int detectives)
	{
		GameState gameState{};

		for (unsigned char i{0}; i < mafia; ++i)
		{
			gameState.m_personStates.push_back(std::make_shared<PersonState>(static_cast<unsigned char>(gameState.m_personStates.size()), Role::Mafia));
		}
		for (unsigned char i{0}; i < detectives; ++i)
		{
			gameState.m_personStates.push_back(std::make_shared<PersonState>(static_cast<unsigned char>(gameState.m_personStates.size()), Role::Detective));
		}
		while (static_cast<int>(gameState.m_personStates.size()) < players)
		{
			gameState.m_personStates.push_back(std::make_shared<PersonState>(static_cast<unsigned char>(gameState.m_personStates.size()), Role::Villager));
		}

		return gameState;
	}

	void _GetWinProbs(int players, int mafia, int detectives, long double& a_villageWinProb, long double& a_mafiaWinProb)
	{
		StateSimulator::GetWinProbs(_CreateGameState(players, mafia, detectives), a_villageWinProb, a_mafiaWinProb);
	}

	void _runCommandInput()
	{
		while(true)
		{
			std::cout << "Welcome to Mafia Simulator 2020: Corona Edition\nEnter the number of total participants: ";
			int totalParticipants;
			std::cin >> totalParticipants;
			if (totalParticipants == 0)
			{
				std::cout << "Mafia wins!\n";
			}
			else
			{
				std::cout << "Enter the number of Mafia members: ";
				int mafias;
				std::cin >> mafias;
				if (mafias == 0)
				{
					std::cout << "Village wins!\n";
				}
				else if(mafias > totalParticipants / 2)
				{
					std::cout << "Mafia wins!\n";
				}
				else
				{
					std::cout << "Enter the number of detectives: ";
					int detectives;
					std::cin >> detectives;
					if (detectives + mafias > totalParticipants)
					{
						std::cout << "There are more detectives and mafia members than total participants. Try again tomorow when you've learned how to read.\n";
					}
					else
					{
						std::cout << "\nCalculating...\n";
						long double villageProb{0};
						long double mafiaProb{0};
						_GetWinProbs(totalParticipants, mafias, detectives, villageProb, mafiaProb);
						std::cout << "Village: ";
						_outputNumberDetailed(villageProb);
						std::cout << "\nMafia: ";
						_outputNumberDetailed(mafiaProb);
						std::cout << "\n";
					}
				}
			}
			std::cout << "Again? (y/n)\n";
			char choice;
			std::cin >> choice;
			if (choice != 'y' && choice != 'Y')
				break;
		}
	}

	void _infiniteCalculation()
	{
		std::ofstream myfile;
		myfile.open("output.txt");

		myfile << "players;mafia;detectives;mafiaWinOdds;villageWinOdds\n";

		myfile.close();
		for (int players{3}; players <= 200; ++players)
		{
			for (int mafia{1}; mafia < players / 2; ++mafia)
			{
				for (int detectives{0}; detectives < players - mafia; ++detectives)
				{
					std::cout << "Calculating " << players << " players " << mafia << " mafia member(s) " << detectives << "detective(s)\n";
					long double villageProb{0};
					long double mafiaProb{0};
					_GetWinProbs(players, mafia, detectives, villageProb, mafiaProb);

					myfile.open("output.txt", std::ios::app);
					myfile << players << ';' << mafia << ';' << detectives << ";=" << mafiaProb << ";=" << villageProb << "\n";
					myfile.close();
				}
			}
		
		}
		
	}

}

int main()
{
	_infiniteCalculation();
}

