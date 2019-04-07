#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <cmath>
#include <string>
#include <vector>

#define data_file "edited.csv"
#define test_file "Test-Set.csv"

struct smallest_data {
    std::string data;
    double distance;
};

std::string used_columns[] = {"country", "year", "age", "sex", "gdp_for_year",  "suicides/100k"};    

std::map<std::string, std::vector<std::string> > load_data(std::string filename);

std::vector<std::string> delimit_string(std::string line, char delimiter);

std::vector<struct smallest_data> determine_smallest(int k, std::vector<struct smallest_data> smallest_distances, std::string value,  double distance);

std::string knn(int k, std::vector<std::string> user_input, std::map<std::string, std::vector<std::string> > table_data);

int main() {
    // Load the data
    std::map<std::string, std::vector<std::string> > table_data = load_data(data_file); 

    // Ask for input
    std::vector<std::string> user_input;
    user_input.push_back("2008");
    user_input.push_back("9.5");
    user_input.push_back("1");
    user_input.push_back("\"1\"");


    std::string result = knn(5, user_input, table_data); 
    std::cout << result << std::endl;
}


std::string knn(int k, std::vector<std::string> user_input, std::map<std::string, std::vector<std::string> > table_data){
    // Smallest distances
    std::vector<struct smallest_data> smallest_distances;
    
    int amount_of_rows = table_data.at(used_columns[0]).size();
    int amount_of_cols = sizeof(used_columns) / sizeof(used_columns[0]);

    // Calculate distances from user input's data to the current data set
    for (int i = 0; i < amount_of_rows; i++){
        double distance;
        for (int j = 1; j < amount_of_cols - 1; j++){
            double tmp =  stod(table_data.at(used_columns[j])[i]) - stod(user_input[j - 1]);
            distance += tmp * tmp;
        }
        // Calculate the euclidean distance 
        distance = sqrt(distance);
        // Factor in the suicides/100k rate into the suicide possibility
        distance = stod(table_data.at(used_columns[amount_of_cols-1])[i]) / distance;
        smallest_distances = determine_smallest(k, smallest_distances, table_data.at(used_columns[0])[i], distance);  
    }

    // Select the most likely result
    int highest_index = -1;
    float highest_val = 0;

    // TODO: MAKE THIS A PROPER SEARCH, THIS IS UGLY, SLOW AND BAD 
    for(int i = 0; i < smallest_distances.size(); i++){
        if(highest_index == -1 || smallest_distances[i].data != smallest_distances[highest_index].data){
            float temp_val = 0;
            for(int j = 0; j < smallest_distances.size(); j++){
                if(smallest_distances[j].data == smallest_distances[i].data){
                    temp_val += smallest_distances[j].distance;
                }
            }
            if(temp_val > highest_val){
                highest_index = i;
                highest_val = temp_val;
            }    
        }
    }

    return smallest_distances[highest_index].data;
}


std::vector<struct smallest_data> determine_smallest(int k, std::vector<struct smallest_data> smallest_distances, std::string value,  double distance){
    int largest_index = -1;
    int replace = false;

    // Check if the vector still has empty indices
    if(smallest_distances.size() >= k){
        for(int i = 0; i < smallest_distances.size(); i ++){
            // Find the index with the largest distance value
            if(largest_index == -1 || smallest_distances[i].distance <= smallest_distances[largest_index].distance){
                largest_index = i;
            }
            // If the new distance is greater than the current distances, then 
            if(distance > smallest_distances[i].distance){
                replace = true;
            }
        }
    // If the vector is empty
    } else {
        replace = true;
    }

    // Remove the value at the largest index
    if(replace){
        // Check if an index needs to be removed
        if(largest_index != -1){
            smallest_distances.erase(smallest_distances.begin() + largest_index);
        }
        // assign new values to a new struct
        struct smallest_data tmp;
        tmp.data = value;
        tmp.distance = distance;

        // push the new struct into the vector
        smallest_distances.push_back(tmp);
    }

    return smallest_distances;
   
}

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