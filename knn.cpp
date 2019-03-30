#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define k 5
#define data_file "Modified-Dataset.csv"

std::map<std::string, std::vector<std::string> > load_data(
    std::string filename);

std::vector<std::string> delimit_string(std::string line, char delimiter);

int main() {
    // Load the data
    std::map<std::string, std::vector<std::string> > table_data = load_data(data_file); 
    
    // Ask for input
    



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
        // Delimit the row
        std::vector<std::string> row_data = delimit_string(row, ',');

        int index = 0;
        // Insert the data into the map
        for(std::string column : columns){
            table_data.at(column).push_back(row_data[index]);
            index++;
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
        if(line_stream.peek() == delimiter || line_stream.peek() == 13){
            // Ignore it
            line_stream.ignore();

            // Add string to vector
            delimited_data.push_back(delimited_val);

            // Reset the column name variable
            delimited_val = "";
        }
    }

    return delimited_data;
}