#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <cmath>
#include <string>
#include <vector>

// The file to read data from
#define data_file "edited.csv"
#define test_file "test.csv"

// Global Variables
std::string used_columns[] = {"country", "year", "age", "sex", "gdp_for_year", "suicides/100k"};    
const int column_amount = 6;

struct neighbors {
    std::string value;
    double distance;
};

// Functions
std::map<std::string, std::vector<std::string> > load_data(std::string filename);
std::vector<std::string> delimit_string(std::string line, char delimiter);
std::string knn(int k, std::map<std::string, std::vector<std::string> > table_data, std::string user_input[column_amount]);
int find_largest_index(int k, struct neighbors closest_neighbors[], double distance);

int main(){
    // Load the data
    std::map<std::string, std::vector<std::string> > table_data = load_data(data_file); 
    std::map<std::string, std::vector<std::string> > test_data = load_data(test_file); 

    // Test for all K values
    for(int k = 1; k < 11; k++){
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
            if(value == knn(k, table_data, tmp_input)){
                correct += 1;
            }
        }
        
        std::cout << "ACCURACY: " << (float)correct / test_data.at(used_columns[0]).size() << " @ K = " << k << std::endl;
    }
    
}

std::string knn(int k, std::map<std::string, std::vector<std::string> > table_data, std::string user_input[column_amount]){
    struct neighbors closest_neighbors[k];

    int col_amount = sizeof(used_columns) / sizeof(used_columns[0]);
    int row_amount = table_data.at(used_columns[0]).size();

    for(int i = 0; i < row_amount; i++){
        double distance = 0.0f;
        // Dont use the last column, used later
        for (int j = 1; j < col_amount - 1; j++){
            double tmp = stod(table_data.at(used_columns[j])[i]) - stod(user_input[j - 1]);
            distance += tmp * tmp;
        }
        // Calculate the euclidean distance 
        distance = sqrt(distance);
        
        // If the suicide per 100k value is 0, dont consider it at all
        if(stod(table_data.at(used_columns[col_amount-1])[i]) != 0){
             // Factor in the suicides/100k rate into the suicide possibility
            distance = distance / stod(table_data.at(used_columns[col_amount-1])[i]);

            // Select smallest here
            int place_index = find_largest_index(k, closest_neighbors, distance);
            if(place_index != -1){
                closest_neighbors[place_index].value = table_data.at(used_columns[0])[i];
                closest_neighbors[place_index].distance = distance;
            }
        }
    }

    // The index of the majority value
    int majority_index = -1;
    // The distance of the smallest value
    double smallest_avg_distance = -1.0f;

    // TODO: Write a better search 
    // Iterate through countries
    for(int i = 0; i < k; i++){
        // Find the amount of time a country occurs
        int occurance = 0;
        double avg_distance = 0.0f;
        for(int j = 0; j < k; j++){
            if(closest_neighbors[i].value == closest_neighbors[j].value){
                occurance += 1;
                avg_distance += closest_neighbors[j].distance;
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
    // for(int i = 0; i < k; i++){
    //     std::cout << closest_neighbors[i].value << " " << closest_neighbors[i].distance << std::endl;
    // }
    return closest_neighbors[majority_index].value;
}

// Find the value to replace in the closest neighbors array
int find_largest_index(int k, struct neighbors closest_neighbors[], double distance){
    int index = -1;
    for(int i = 0; i < k; i++){
        if(closest_neighbors[i].value == ""){
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