#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <pthread.h>

// How many neighbors to consider
#define K 5

// The file to read data from
#define data_file "edited.csv"
#define test_file "test.csv"

// The # of threads used
#define MAX_THREADS 8

// The columns to take into account and its size  
const int column_amount = 6;
std::string used_columns[] = {"country", "year", "age", "sex", "gdp_for_year", "suicides/100k"};  

// All of the data extracted from a csv
std::map<std::string, std::vector<std::string> > table_data;

// A struct to define a neighbor
struct neighbors {
    std::string group; // The classification of a value
    double distance; // The distance from the value to the input
};

// A struct containing all of the data needed for a thread
struct thread_data {
    int thread_id; // The ID of a thread
    std::string user_input[column_amount]; // The input passed
    struct neighbors closest_neighbors[K]; // A place to store the neighbors
};

// Define functions used
void* find_closest(void* data);
std::string knn(std::string user_input[column_amount]);
int find_largest_index(struct neighbors closest_neighbors[], double distance);
std::map<std::string, std::vector<std::string> > load_data(std::string filename);
std::vector<std::string> delimit_string(std::string line, char delimiter);

int main(){
    // Load the data
    table_data = load_data(data_file); 
    std::map<std::string, std::vector<std::string> > test_data = load_data(test_file); 

    // The amount of values tested correctly
    int correct = 0;

    // Iterate through the test values 
    for(int i = 0; i < test_data.at(used_columns[0]).size(); i++){
        // Get the answer (first column)
        std::string value = test_data.at(used_columns[0])[i];

        // Get the other inputs (other columns)
        int input_col_size = sizeof(used_columns) / sizeof(used_columns[0]);
        std::string tmp_input[input_col_size - 2];
        for(int j = 1; j < input_col_size - 1; j++){
            tmp_input[j - 1] = test_data.at(used_columns[j])[i];
        }

        // Check if the result of knn is the same as the real answer
        if(value == knn(tmp_input)){
            correct += 1;
        }
    }
    
    std::cout << "ACCURACY: " << (float)correct / test_data.at(used_columns[0]).size() << " @ K = " << K << std::endl;
}

std::string knn(std::string user_input[column_amount]){
    // Define the array to hold the threads
    pthread_t threads[MAX_THREADS]; 
    // Define the attribute for the threads (mainly joinable)
    pthread_attr_t thread_attr;  
    // Define the array to hold the variables for each thread
    struct thread_data data[MAX_THREADS];

    // Initalize the attribute and set it to JOINABLE
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

    for(int thread_id = 0; thread_id < MAX_THREADS; thread_id++){
        // Set the thread id
        data[thread_id].thread_id = thread_id;
        // Copy the user's data
        for(int i = 0; i < column_amount - 2; i++){
            data[thread_id].user_input[i] = user_input[i];
        }

        // Run the thread
        pthread_create(&threads[thread_id], &thread_attr, find_closest, (void*)&data[thread_id]);
    }

    // Remove the attribute to free memory
    pthread_attr_destroy(&thread_attr);
    // Define the final K closest neighbors
    struct neighbors final_closest_neighbors[K];

    for (int thread_id = 0; thread_id < MAX_THREADS; thread_id++) {
        // Join all threads back
        pthread_join(threads[thread_id], NULL);

        // Determine what neighbors to keep 
        for(int i = 0; i < K; i++){
            // Select smallest here
            int place_index = find_largest_index(final_closest_neighbors, data[thread_id].closest_neighbors[i].distance);

            // Swap neighbors
            if(place_index != -1){
                final_closest_neighbors[place_index].group = data[thread_id].closest_neighbors[i].group;
                final_closest_neighbors[place_index].distance = data[thread_id].closest_neighbors[i].distance;
            }
        }
    }

    // The index of the majority value
    int majority_index = -1;
    // The distance of the smallest value
    double smallest_avg_distance = -1.0f;

    // TODO: Write a better search 
    // Iterate through countries
    for(int i = 0; i < K; i++){
        // Find the amount of time a country occurs
        int occurance = 0;
        double avg_distance = 0.0f;
        for(int j = 0; j < K; j++){
            if(final_closest_neighbors[i].group == final_closest_neighbors[j].group){
                occurance += 1;
                avg_distance += final_closest_neighbors[j].distance;
            }
        }
        // Calculate the average distance
        avg_distance = avg_distance / occurance;

        // If its the smallest average distance, select it 
        if(avg_distance < smallest_avg_distance || smallest_avg_distance == -1.0f){
            smallest_avg_distance = avg_distance;
            majority_index = i;
        }
    }

    // Print the results - DEBUG USE
    // for(int i = 0; i < K; i++){
    //     std::cout << final_closest_neighbors[i].group << " " << final_closest_neighbors[i].distance << std::endl;
    // }
    return final_closest_neighbors[majority_index].group;
}

void* find_closest(void* data){
    // Get the thread's data
    struct thread_data* parameters = (struct thread_data*) data;

    // Get the input data's dimensions
    int col_amount = sizeof(used_columns) / sizeof(used_columns[0]);
    int row_amount = table_data.at(used_columns[0]).size();

    // Iterate through the data
    for(int i = parameters->thread_id; i < row_amount; i+=MAX_THREADS){
        double distance = 0.0f;

        // Dont use the last column, used later
        for (int j = 1; j < col_amount - 1; j++){
            double tmp = stod(table_data.at(used_columns[j])[i]) - stod(parameters->user_input[j - 1]);
            distance += tmp * tmp;
        }
        // Calculate the euclidean distance 
        distance = sqrt(distance);
        
        // If the suicide per 100k value (last column) is 0, dont consider it at all
        if(stod(table_data.at(used_columns[col_amount-1])[i]) != 0){
             // Factor in the suicides/100k rate into the suicide possibility
            distance = distance / stod(table_data.at(used_columns[col_amount-1])[i]);

            // Find the value to swap in the array
            int place_index = find_largest_index(parameters->closest_neighbors, distance);

            if(place_index != -1){
                parameters->closest_neighbors[place_index].group = table_data.at(used_columns[0])[i];
                parameters->closest_neighbors[place_index].distance = distance;
            }
        }
    }
}

// Find the value to replace in the closest neighbors array
int find_largest_index(struct neighbors closest_neighbors[], double distance){
    int index = -1;
    for(int i = 0; i < K; i++){
        if(closest_neighbors[i].group == ""){
            index = i;
            break;
        } else if(distance < closest_neighbors[i].distance && (index == -1 || closest_neighbors[i].distance > closest_neighbors[index].distance)){
            index = i;
        }
    }
    return index;
}

// Places csv data into a map
std::map<std::string, std::vector<std::string> > load_data(
    std::string filename) {
    // The input stream from the file
    std::ifstream data_stream;
    // Open the file
    data_stream.open(filename);

    // The map to hold all of the data
    std::map<std::string, std::vector<std::string> > table_data;
    // The tmp string to hold the line
    std::string row;
    // Read the column row
    std::getline(data_stream, row);
    // Delimit the row
    std::vector<std::string> columns = delimit_string(row, ',');
    
    // Initalize the map
    for(std::string column : columns){
        // Create the vector to hold the column's data
        std::vector<std::string> column_vector;
        // Insert into the map
        table_data.insert(make_pair(column, column_vector));
    }

    // Fill the columns
    while (!data_stream.eof()) {
        std::getline(data_stream, row);
        
        // Prevent if the line is empty
        if(row != ""){
            // Delimit the row
            std::vector<std::string> row_data = delimit_string(row, ',');
            
            int index = 0;
            // Insert the data into the map
            for(std::string column : columns){
                table_data.at(column).push_back(row_data[index]);
                index++;
            }
        }
    }

    // Close the stream and return
    data_stream.close();

    return table_data;
}

// Delimites a string into a vector
std::vector<std::string> delimit_string(std::string line, char delimiter){
    // A vector to hold the data
    std::vector<std::string> delimited_data;

    // Use a stringstream to split that data
    std::stringstream line_stream(line);

    // A string to hold the final column name
    std::string delimited_val;
    // A temp char to hold the character from the string
    char tmp;
    
    while(line_stream >> tmp){
        delimited_val += tmp;

        // If there is a comma next
        if(line_stream.peek() == delimiter || line_stream.peek() == 13 || line_stream.peek() == -1){
            // Ignore it
            line_stream.ignore();

            // Add string to vector
            delimited_data.push_back(delimited_val);

            // Reset the column name variable
            delimited_val = "";
        } else if((int)line_stream.peek() == 32){
            // Add the space manually since it is not done
            delimited_val += " ";
        }
    }

    return delimited_data;
}   