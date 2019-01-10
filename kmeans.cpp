/*
Cory Mollenhour
CSCI 4350 - Joshua Phillips
Open Lab 4
Due: 12/4/2018 11:00 PM
*/


#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#define MAX_ITERATIONS 10

using namespace std;


//Nodes store position data of dataPoints for any number of dimensions
struct Node {
	vector<double> dataPoints;
	int classLabel;
	bool isCenter = false;
};

//Centroids store data point positions as well as a collection of Nodes in the cluster and the count of each class
struct Centroid {
	vector<double> cPoint;
	vector<Node *> clusterNodes;
	vector<int> classCounts;
	int classLabel;
};


vector<Node*> trainingData;
vector<Node*> testingData;
vector<Centroid*> centroidList;


vector<double> maxFeatures;

//Prototypes
double euclideanDistance(Node * data, Centroid * cent);
int  getMajorityClass(Centroid * cent);
int predictClasses(vector<Centroid*> centroidList, vector<Node*> testing);
int getNearest(vector<Centroid*> centroidList, Node * node);

//Global Variables
int n;
int seed;
int clusters;
int features;
int testLength;
int trainingLength;
int highestClass = 0;

int main(int argc, char* argv[])
{

	string training;
	string testing;
	string line;

	if (argc == 6) {
		seed = atoi(argv[1]); 
		clusters = atoi(argv[2]); //input training data filename
		features = atoi(argv[3]); //the number of real-valued features in the data set
		training = argv[4]; //input testing data filename
		testing = argv[5]; //input testing data filename
	}
	else {
		cout << "Needs 5 arguments" << endl;
		return 0;
	}
	
	
	srand(seed);
	//Read in file and create a vector of nodes
	//Record the features as dataPoints
	
	//initialize all maxFeatures to 0 for finding range
	for (int i = 0; i < features; i++)
		maxFeatures.push_back(0);
	
	//Read in Data for Training
	ifstream myfile(training);
	if (myfile.is_open())
	{
		while (getline(myfile, line)){
			string temp;
			Node * newNode = new Node();
			istringstream iss(line);
			
			//create m number of Nodes with dataPoints where m = Lines of data
			for (int j = 0; j < features+1; j++) {
				getline(iss, temp, ' ');
				if (j < features) {
					newNode->dataPoints.push_back(stof(temp));
					if (newNode->dataPoints[j] > maxFeatures[j])
						maxFeatures[j] = newNode->dataPoints[j];
				} else {
					newNode->classLabel = stoi(temp);
					if (highestClass < stoi(temp)) {
						highestClass = stoi(temp);
					}
				}
			}

			trainingData.push_back(newNode);
			trainingLength++;
		}
		myfile.close();
	}
	else cout << "Unable to open training file";

	//Read in Data for Training
	ifstream myfile2(testing);
	if (myfile2.is_open())
	{
		while (getline(myfile2, line)){
			string temp;
			Node * newNode = new Node();
			istringstream iss(line);

			//create m number of Nodes with dataPoints where m = Lines of data
			for (int j = 0; j < features + 1; j++) {
				getline(iss, temp, ' ');
				if (j < features) {
					newNode->dataPoints.push_back(stof(temp));
					if (newNode->dataPoints[j] > maxFeatures[j])
						maxFeatures[j] = newNode->dataPoints[j];
				} else {
					newNode->classLabel = stoi(temp);
				}
			}

			testingData.push_back(newNode);
			testLength++;
		}
		myfile2.close();
	} else cout << "Unable to open testing file";
	
	//Create n number of centroids, where n = features
	for (int i = 0; i < clusters; i++) {
		Centroid * cent = new Centroid();
		srand(seed);
		int random = rand() % trainingLength;
		while (trainingData[random]->isCenter) { //Don't reuse a datapoint for centroid location
			random = rand() % trainingLength;
		}

		for (int j = 0; j < features; j++)
			cent->cPoint.push_back(trainingData[random]->dataPoints[j]);
		
		cent->classLabel = trainingData[random]->classLabel;
		trainingData[random]->isCenter = true;
		centroidList.push_back(cent);
	}
	
	//Initialize classCount vectors for trackign amt of each class
 	for (int i = 0; i < clusters; i++) 
		for (int j = 0; j < highestClass+1; j++)
			centroidList[i]->classCounts.push_back(0);

	bool hasChange = true;
	
	int iterations = 0;
	while (hasChange) {
		hasChange = false;
		//determine the distance of all data points to all centroids
		vector<Centroid*> * newCentroidList = new vector<Centroid*>;
		vector<vector<double>> allDistances;
		for (int k = 0; k < clusters; k++) {
			vector<double> distance;
			for (int j = 0; j < trainingLength; j++)
				distance.push_back(euclideanDistance(trainingData[j], centroidList[k]));

			allDistances.push_back(distance);
		}
		for (int i = 0; i < clusters; i++)
			centroidList[i]->clusterNodes.clear();

		for (int i = 0; i < clusters; i++)
			for (int j = 0; j < highestClass+1; j++)
				centroidList[i]->classCounts[j] = 0;

		//Find shortest Distance
		for (int j = 0; j < trainingLength; j++) {
			double shortest = 10000000.0;
			int centroidNum = 0;
			
			for (int k = 0; k < clusters; k++) {
				if (allDistances[k][j] <= shortest) {
					shortest = allDistances[k][j];
					centroidNum = k;
				}
			}
			//Populate clusterNodes centroidNum with closest nodes j
			centroidList[centroidNum]->clusterNodes.push_back(trainingData[j]);
			//Add 1 to the node classNumber when adding node to clusterNodes
			int selectedClass = trainingData[j]->classLabel;
			centroidList[centroidNum]->classCounts[selectedClass]++;
		}

		//Move centroid by averaging and creating new Centroid with the avg location
		for (int i = 0; i < clusters; i++) {						//For every clusterNodes
			int clusterSize = centroidList.at(i)->clusterNodes.size();	//Get clusterNodes size
			Centroid * newCentroid = new Centroid();
			
			//Get average
			for (int k = 0; k < features; k++) {	
				double sum = 0;
				double avg = 0;				//And for each feature (x,y,z...)
				for (int j = 0; j < clusterSize; j++) {				
					sum += (centroidList[i]->clusterNodes[j]->dataPoints[k]);	//add every node in clusterNodes to Sum	
				}	
				avg	= (sum / clusterSize);									// And get average
				
				//Make new centroid at new average location
				double originalLocation = centroidList[i]->cPoint[k];
				newCentroid->cPoint.push_back(avg); // newPoint[0] = w, newPoint[1] = x, newPoint[2] = y, newPoint[3] = z... 
				if (originalLocation != avg) {
					hasChange = true;
				}
			}

			//assign nodes to new centroid after moving centroid
			newCentroid->clusterNodes = centroidList[i]->clusterNodes; //Add node to new clusterNodes from original centroid list
			newCentroid->classCounts = centroidList[i]->classCounts;
			newCentroid->classLabel = getMajorityClass(centroidList[i]);
			newCentroidList->push_back(newCentroid);
		}

		centroidList.clear();
		for (int i = 0; i < clusters; i++) {
			centroidList.push_back(newCentroidList->at(i));
		}
		newCentroidList->clear();
		if (hasChange == false || iterations == MAX_ITERATIONS)
			break;
		iterations++;
			
	}

	int predictionsCorrect = predictClasses(centroidList, testingData);

	cout << predictionsCorrect << endl;
	
	return 0;
}

//Finds the nodes that are nearest to the centroid specified
int getNearest(vector<Centroid*> centroidList, Node * node) {
	double lowest = 1000000.0;
	double distance;
	int index = -1;
	for (int i = 0; i < clusters; i++) {
		distance = euclideanDistance(node, centroidList[i]);
		if (distance <= lowest) {
			lowest = distance;
			index = i;
		}
	}
	return index;
}

//Return number of correct predictions
int predictClasses(vector<Centroid*> centroidList, vector<Node*> testing) {
	
	int correct = 0;
	
	for (int i = 0; i < testLength; i++) {
		int nearestCluster = getNearest(centroidList, testing[i]);
		int prediction = centroidList[nearestCluster]->classLabel;
		int actualClass = testing[i]->classLabel;
		if (prediction == actualClass)
			correct++;
	}
	return correct;
}

//Get Euclidean Distance from a node, to a centroid
double euclideanDistance(Node * data, Centroid * cent) {
	double sum = 0.0;
	for (int i = 0; i < features; i++)
		sum += (pow((cent->cPoint[i] - data->dataPoints[i]), 2.0)); //Sum up (y - x)^2 for every feature (dimension)

	return sqrt(sum);
}

//Get the class majority within a centroid
int  getMajorityClass(Centroid * cent) {
	int max = 0;
	int highestCount = 0;
	if(cent->clusterNodes.size() > 0){
		for(int i=0; i<highestClass+1; i++)
			if (cent->classCounts[i] > max) {
				max = cent->classCounts[i];
				highestCount = i;
			}
		return highestCount;
	}
}
