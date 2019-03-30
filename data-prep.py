import os
import csv

with open("master.csv", encoding='utf-8-sig') as csv_file:
    reader = csv.reader(csv_file, delimiter=',')
    headers = []
    data = {}

    # Load the data into the dictonary
    header_row = True
    for row in reader:
        # Read the first row into the header array
        if(header_row == True):
            headers = row

            # Init the data container
            for header in headers:
                data[header] = [];

            header_row = False
        else:
            for col, header in zip(row, headers):
                if(header == 'age'):
                    age_val = 0.0

                    if(len(col) > 4):
                        age_val = (int(col[0:2]) + int(col[3:len(col)])) / 2
                    elif(len(col) == 4):
                        age_val = (int(col[0:1]) + int(col[2: len(col) ])) / 2
                    else:
                        age_val = (100 + int(col[0:len(col) - 1])) / 2

                    data[header].append(age_val)
                else:
                    data[header].append(col)


    # Dictonary that will be written out to the final file
    final = {}
    # The data points we want
    final_headers = ['country', 'year', 'age', 'sex', 'suicides/100k']
    for final_header in final_headers:
        final[final_header] = data[final_header]

    # Write to the file
    with open("edited.csv", "w+") as out_file:
        writer = csv.writer(out_file, delimiter=',')
        writer.writerow(final_headers) # Write the headers

        # Loop through all the rows
        for i in range(len(final[final_headers[0]])):
            row = []
            # For each column add it's data to the array
            for final_header in final_headers:
                row.append(final[final_header][i])

            # Write the row
            writer.writerow(row)