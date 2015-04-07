#include "StdAfx.h"
#include "Guesser.h"
#include <algorithm>    // std::sort

#include "SecretKeeper.h"

//utility function
int factorial(int n)
{
	if(n==0)
		return 1;
	else
		return n*factorial(n-1);
}

Guesser::Guesser(const int size, const int maxNumber):
	m_nSecretSize(size),
	m_nMaxNumber(maxNumber),
	m_nRightlytPlaced(0),
	m_bDebug(false)
{
	m_rightlyPlaced = new int[m_nSecretSize];//R
	for(int i=0;i<m_nSecretSize;i++)
	{
		m_rightlyPlaced[i] = -1;
	}
}

Guesser::~Guesser(void)
{
	for(int i=0;i<(int)m_tryArrays.size();i++)
	{
		delete[] m_tryArrays[i];
	}

	for(int i=0;i<(int)m_resultArrays.size();i++)
	{
		delete[] m_resultArrays[i];
	}

	delete[] m_rightlyPlaced;
}

int *Guesser::Try(void)
{
	const int currentInteger = (int)m_tryArrays.size();
	int* currentTry = new int[m_nSecretSize];
	//RAII, default to -1 or values we know are well placed
	for(int i=0;i<m_nSecretSize;i++)
	{
		currentTry[i]=m_rightlyPlaced[i];
	}

	//If we didn't already try all 'colors' (0to maxNumber) or we already found all colors used!
	if(currentInteger<=m_nMaxNumber && m_nSecretSize!=numberColorsFound())
	{
		//Change improper values with the current 'color' to try
		for(int i=0;i<m_nSecretSize;i++)
		{
			if(currentTry[i]==-1)
				currentTry[i] = currentInteger;
		}

		//Then replace the currentValues by values for which we didn't find a place yet.
		//TODO IMPROVE IT SO WE DON'T PLACE A VALUE AT THE SAME BAD SPOT ON DIFFERENT ROUNDS!!!!!
		//For each 'color' we want to place
		for(auto it = m_toPlace.begin(); it<m_toPlace.end();it++)
		{
			findSpot(*it, currentTry, currentInteger);
		}
	}
	
	//Otherwise we know the list of colors that compose the secret! so there only needs to permute colors we aren't sure about there places.
	else
	{
		//Then put the other values in places that would change
		//TODO improve this algo
		for(auto it=m_toPlace.begin();it!=m_toPlace.end();it++)
		{
			findSpot(*it, currentTry, -1);
		}

		//Check that we didn't already try this combination, if we did shuffle it
		//After all, once we know the 6 values of the code, there is maximum 720 combinations!
		//Put values in an order that as never been performed before (the number of element to place can give the order of the combinations to test!)
		//It's possible to enumerate the permutations, like Heap's algorithm
		int numberElementsToPlace = m_toPlace.size();
		int numberPermutations = factorial(numberElementsToPlace);//todo, check that there isn't any overflow here :)
		printf("Debug: we might have to shuffle, and there are just %d possibilities, trolol\n", numberPermutations);
		int* shuffleArray = new int[numberElementsToPlace];

		//With Heap's algorithm we shuffle the last array we cooked!
		int index=0;
		std::sort(m_toPlace.begin(), m_toPlace.end());//starts by sorting the vector, useless?! cause we want a sorted vector for next_permutation!!
		for(auto it=m_toPlace.begin();it!=m_toPlace.end();it++)
		{
			shuffleArray[index]=*it;
			index++;
		}

		//Shuffle the numbers!
		int permutationDecimalNumber = 0;//0; 0 is identity lol, we already spotted 2 times that it doesn't work: TODO avoid alreadyTried twice for same array (above and just below)
		//We want to stop when !alreadyTried && cachedguessfits or if permutationDecimalNumber==numberPermutations,so we continue when
		while( (alreadyTried(currentTry) || !cachedGuessFits(currentTry)) && permutationDecimalNumber<numberPermutations)
		{
			//Recopies the values we know fit (always start with the currentTry full of -1 for bad values)
			for(int i=0;i<m_nSecretSize;i++)
			{
				currentTry[i]=m_rightlyPlaced[i];
			}

			std::next_permutation(shuffleArray,shuffleArray+(int)m_toPlace.size());//a big thank you to the standardization committee <3 <3
			
			//printf("just permuted to state: ");
			//debugArrays(shuffleArray,numberElementsToPlace);

			//copy back to the currentarray
			for(int shuffleIndex=0;shuffleIndex<numberElementsToPlace;shuffleIndex++)
			{
				int currentArrayIndex=0;
				while(currentTry[currentArrayIndex]!=-1 )
				{
					currentArrayIndex++;
				}
				currentTry[currentArrayIndex]=shuffleArray[shuffleIndex];
			}
		
			permutationDecimalNumber++;
		}//end while loop
		if(permutationDecimalNumber==numberPermutations)
			printf("ERROR: detected that we tried all combinations already oO\n");

		delete[] shuffleArray;
	}//end else loop

	debugArrays(currentTry);

	m_tryArrays.push_back(currentTry);
	return currentTry;
	
}

void Guesser::Feedback(int nR, int nB)
{
	int* result = new int[2];
	result[0] = nR;
	result[1] = nB;

	std::cout << nR << " " << nB << std::endl;

	const int currentValue = m_resultArrays.size();
	m_resultArrays.push_back(result);//currentValue parameter 

	//Now we compute the feedback!

	//2nd way to detect a value rightly placed
	//When there is 0 nB, we know all previous values that doesn't equal the current value being tested are rightly placed!
	if(nB==0 && m_toPlace.size()>0)
	{
		const int lastLigne = m_tryArrays.size() -1;
		if(currentValue!=lastLigne)//check
		{
			printf("ERROR there is not the same size for tryArrays and resultArrays oO");
		}

		for(int i=0; i<m_nSecretSize; i++)
		{
			if( m_rightlyPlaced[i]==-1 && m_tryArrays[lastLigne][i] != currentValue)
			{
				m_rightlyPlaced[i]=m_tryArrays[lastLigne][i];
				printf("By 2nd deduction (nB being 0) we found that color %d lies in column %d\n",m_rightlyPlaced[i],i);
			}
		}
		//When nB equals 0, there are no current value to place (until we had those of the current value tested like below)
		m_toPlace.clear();//empties the vector
	}
	//

	//Update our array of values to place and values rightly placed!
	//if no R or B before, and now then we found a value to place!
	int previousScore = 0;//Score here is nB+nR
	if(currentValue>0)
		previousScore=RplusB(currentValue-1);
	int currentScore = RplusB(currentValue);
	bool alreadyAdded = (currentValue==m_nMaxNumber);//thanks to an optimization cf below
	if(previousScore<currentScore && !alreadyAdded)
	{
		printf("there is/are %d occurrence(s) of: %d in secret\n",currentScore-previousScore,currentValue);
		for(int i=0;i<(currentScore-previousScore);i++)
		{
			m_toPlace.push_back(currentValue);
		}
	}
	
	//1st way to detect a value rightly placed
	//If there is a value misplaced that as been tested at all places without any R, then the right place for it is the last one!
	//For each color pending to be placed
	for(auto it = m_toPlace.begin(); it!= m_toPlace.end(); )//it++ removed it, see below
	{
		std::vector<int> columnCandidates;//list of potential rightly placed (R) for this color in the secret, if only one in the vector, we found where it lays!!

		//for each column
		for(int i=0; i<m_nSecretSize; i++)
		{
			bool isPotentialGoodColumnForThisColor = true;
			//for each line
			for(int j=0; j< (int)m_resultArrays.size();j++)
			{
				//IF there is no R (available for this value) and we tried it here already, it isn't a good spot to retry
				//WARNING: This works because we always put the number we are sure about in our tries!
				//if(m_resultArrays[j][0]==0 && m_tryArrays[j][i]==*it)
				if(m_resultArrays[j][0]<=m_nRightlytPlaced && m_tryArrays[j][i]==*it)
				{
					isPotentialGoodColumnForThisColor = false;
				}
			}
			if(isPotentialGoodColumnForThisColor)
			{
				columnCandidates.push_back(i);
			}
		}
		
		//Whether we delete an element from our loop
		if(columnCandidates.size()==1)
		{
			//We will then remove it from misplaced, and add to rightly placed at the good spot
			printf("By deduction we found that color %d lies in column %d\n",*it,columnCandidates.at(0));

			if(m_rightlyPlaced[columnCandidates.at(0)]!= -1)
			{
				printf("ERROR there was already a value (%d) in m_rightlyPlaced array at index ().\n",m_rightlyPlaced[columnCandidates.at(0)],columnCandidates.at(0));
			}
			
			m_rightlyPlaced[columnCandidates.at(0)] = *it;

			m_nRightlytPlaced++;

			printf("DEBUG removing value %d at index %d\n",m_rightlyPlaced[columnCandidates.at(0)],columnCandidates.at(0));
			it = m_toPlace.erase(it);//this is fucking not recommended
		}
		//or we increment the iterator
		else
		{
			it++;
		}
	}

	//Finally: if we tried the penultimate, we know the last values to place are for the ultimate
	if(currentValue+1==m_nMaxNumber)
	{
		int nFound = numberColorsFound();
		printf("penultimate: there is/are %d occurrence(s) of: %d in secret\n",m_nSecretSize-nFound, m_nMaxNumber);
		for(int i=0;i<(m_nSecretSize-nFound);i++)
		{
			m_toPlace.push_back(m_nMaxNumber);
		}
	}
	
}

int Guesser::RplusB(int n)
{
	//assert on n?
	if(n>=0 && n<=(int)m_resultArrays.size())
		return m_resultArrays[n][0]+m_resultArrays[n][1];
	else
	{
		printf("ERROR index out of bound\n");//abort();//make it crash baby
		return 0;
	}
}

//Give the number of colors that we know are in the code
int Guesser::numberColorsFound()
{
	int n=(int)m_toPlace.size();
	for(int i=0; i<m_nSecretSize; i++)
	{
		if(m_rightlyPlaced[i] != -1)
		{
			n++;
		}
	}
	return n;
}

void Guesser::debugArrays(int* myArray, int taille)
{
	if(taille==-1)
		taille=m_nSecretSize;

	if(myArray==NULL)
		return;

	for(int i=0; i<taille;i++)
	{
		std::cout << myArray[i] << " ";
		if(myArray[i]<0)//debug
			printf("WTF\n");
	}

	std::cout << std::endl;
}

//Check if the value has been in the column (except for the ligne when we wanted to know the number of occurrences of that value)
//For instance for value 0, there is no need to check the 0th ligne, cause we only had this value there!
//Note that before the value has been tested, there is no occurrence of this value before!
bool Guesser::hasNeverBeenInColumn(int value, int column)
{
	if(column<0 || column>=m_nSecretSize)
		printf("ERROR in hasNeverBeenInColumn column not included in array\n");
	
	bool isFree = true;
	const int currentInteger = (int)m_tryArrays.size();

	//Start from the ligne after the first time we try this value!
	for(int ligne=value+1;ligne<currentInteger;ligne++)
	{
		if(m_tryArrays[ligne][column]==value)
			isFree = false;
	}
	
	return isFree;
}

void Guesser::findSpot(int value, int* currentTry, int defaultValue)
{
	bool foundSpot = false;
	int i=0;
	int version=0;//Has our previous criteria might be too hard for the value, we give it another chance with lesser criteria

	const int lastLigne = m_tryArrays.size() -1;

	while(!foundSpot && version<3)
	{
		i=0;
		while(!foundSpot && i< m_nSecretSize)
		{
			if(currentTry[i]==defaultValue)//We want to compare it to the default value, which can also be -1
			{
				//If the last color we tried was THIS color, then it's ok to put it again on the same spot!
				//if(currentInteger-1==*it || m_tryArrays[currentInteger-1][i]!=*it)
				//If we couldn't find a spot during two previous versions, then just put this value where we can! better placed than not :)
				if(lastLigne==value || version==0?hasNeverBeenInColumn(value,i):m_tryArrays[lastLigne][i]!=value || version==2)
				{
					foundSpot = true;
					currentTry[i]=value;
				}
			}
			i++;
		}
		version++;
	}

	if(!foundSpot)
		printf("ERROR didn't find a spot for color %d\n", value);
}

//compares m_tryArrays with the currentTryArray, kind of return  Exists i so that m_resultArrays[i] semantiqualy != currentTryArray
//TODO
bool Guesser::alreadyTried(int*currentTryArray)
{
	for(int i=0; i!=m_resultArrays.size(); i++)
	{
		bool areSame = true;
		int j=0;
		while(areSame && j<m_nSecretSize)
		{
			if(m_tryArrays[i][j]!=currentTryArray[j])
			{
				areSame = false;
			}
			j++;
		}
		if(areSame)
		{
			if(m_bDebug)
			{
				printf("DEBUG we already tried that combination ");
				debugArrays(currentTryArray);
			}
			
			return true;
		}
	}
	return false;
}

//perform a simulation of all previous tries assuming that the currentTryArray is the key,
//and compare the answers we got with 
bool Guesser::cachedGuessFits(int*currentTryArray)
{
	//Avoid code duplication, we create our own guesser and assign it the value we think could be the key, just to compare the (R,B) results
	std::unique_ptr<SecretKeeper> subordinate;
	subordinate.reset(new SecretKeeper(m_nSecretSize, m_nMaxNumber, currentTryArray));

	//For each line of tryArray, determine if R,B is the same as it would be
	for(int ligne=0;ligne<(int)m_tryArrays.size();ligne++)
	{
		int nR=0;//number of elements righlty placed
		int nB=0;//number of elements misplaced
		std::tie(nR, nB)= subordinate->Guess(m_tryArrays[ligne]);//auto result 

		if(m_resultArrays[ligne][0]!=nR || m_resultArrays[ligne][1]!=nB)
		{
			if(m_bDebug)
			{
				printf("Debug currentTry try can't fit:");
				debugArrays(currentTryArray);
				printf("we would have (nR,nB)= %d,%d and we had %d,%d on line %d\n",nR,nB,m_resultArrays[ligne][0], m_resultArrays[ligne][1],ligne );
			}
			
			return false;
		}
	}
	return true;
}
