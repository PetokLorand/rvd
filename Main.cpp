#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <functional>
#include <filesystem>

template <typename TInputIt, typename TOutputIt, typename TDelimiter>
TOutputIt split(TInputIt first, TInputIt last, TOutputIt output, TDelimiter delim)
{
    auto isDelim = [&delim](const auto& value) { return value == delim; };
    
    TInputIt it = first;
    while (it != last)
    {
        first = std::find_if_not(first, last, isDelim);
        it = std::find_if(first, last, isDelim);
        if (first != it) *output++ = { first, it };
        first = it + 1;
    }

    return output;
}

struct Employee
{
    std::string m_name = "";
    double m_salary = 0.0;
    int m_availableVacationDays = 0;
    int m_requiredVacationDays = 0;
    int m_adjustedVacationDays = 0;

    bool operator==(const Employee& rhs) 
    { 
        return m_name == rhs.m_name && m_salary == rhs.m_salary && m_availableVacationDays == rhs.m_availableVacationDays; 
    }

    friend std::ostream& operator<<(std::ostream& os, const Employee& employee)
    {
        os << employee.m_name << ',';
        os << employee.m_salary << ',';
        os << employee.m_availableVacationDays << ',';
        os << employee.m_requiredVacationDays << ',';
        os << employee.m_adjustedVacationDays << ',';
        os << employee.m_requiredVacationDays - employee.m_adjustedVacationDays;

        return os;
    }

    void parse(const std::string& line)
    {
        std::vector<std::string> tokens;
        split(line.begin(), line.end(), std::back_inserter(tokens), ',');

        if (tokens.size() != 3)
        {
            std::cout << "Employee parsing error!";
            return;
        }

        m_name = tokens[0];
        m_salary = std::stod(tokens[1]);
        m_availableVacationDays = std::stoi(tokens[2]);
    }
};

int calculatePercentage(const std::vector<Employee>& employees, double targetValue)
{
    double actualValue = 0.0;
    int percentage = 0;

    auto lambda = [&percentage](const Employee& employee)
    { 
        return static_cast<double>(employee.m_availableVacationDays) * percentage / 100 * employee.m_salary; 
    };

    while (actualValue < targetValue)
    {
        actualValue = std::transform_reduce(employees.begin(), employees.end(), 0.0, std::plus<double>(), lambda);
        ++percentage;
    }

    return percentage;
}

void updateEmployees(std::vector<Employee>& employees, int percentage)
{
    auto update = [percentage](Employee& employee) 
    { 
        employee.m_requiredVacationDays = std::ceil(static_cast<double>(employee.m_availableVacationDays) * percentage / 100);
        employee.m_adjustedVacationDays = employee.m_requiredVacationDays;
    };

    std::for_each(employees.begin(), employees.end(), update);
}

double calculateValue(const std::vector<Employee>& employees)
{
    auto lambda = [](const Employee& employee)
    { 
        return employee.m_adjustedVacationDays * employee.m_salary;
    };

    return std::transform_reduce(employees.begin(), employees.end(), 0.0, std::plus<double>(), lambda);
}

double adjustRequiredVacationDays(std::vector<Employee>& employees, double targetValue, int percentage)
{
    auto calculateCeilError = [percentage](const Employee& employee) -> double
    {
        double value = static_cast<double>(employee.m_availableVacationDays) * percentage / 100;
        return std::ceil(value) - value;
    };

    auto comparator = [calculateCeilError](const Employee& lhs, const Employee& rhs)
    {
        return calculateCeilError(lhs) > calculateCeilError(rhs);
    };

    std::vector<Employee> sortedEmployees(employees.begin(), employees.end());

    std::sort(sortedEmployees.begin(), sortedEmployees.end(), comparator);

    double adjustedValue = calculateValue(sortedEmployees);
    double previousAdjustedValue;

    auto it = sortedEmployees.begin();
    for (it = sortedEmployees.begin(); it != sortedEmployees.end() && adjustedValue >= targetValue; ++it)
    {
        previousAdjustedValue = adjustedValue;
        it->m_adjustedVacationDays--;
        adjustedValue = calculateValue(sortedEmployees);
    }

    (it - 1)->m_adjustedVacationDays++;

    auto lambda = [&sortedEmployees](Employee& employee)
    {
        auto it = find(sortedEmployees.begin(), sortedEmployees.end(), employee);
        employee.m_requiredVacationDays = it->m_requiredVacationDays;
        employee.m_adjustedVacationDays = it->m_adjustedVacationDays;
    };

    std::for_each(employees.begin(), employees.end(), lambda);

    return previousAdjustedValue;
}

std::istream& operator>>(std::istream& is, std::vector<Employee>& employees)
{
    std::string line;
    while (std::getline(is, line))
    {
        Employee employee;
        employee.parse(line);        
        employees.push_back(employee);
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const std::vector<Employee>& employees)
{
    for (std::size_t idx = 0; idx < employees.size(); ++idx) 
    { 
        os << employees[idx];
        
        if (idx < employees.size() - 1)
        {
            os << std::endl;
        } 
    }; 
    
    return os;
}

int main(int argc, char** args)
{
    if (argc < 3)
    {
        std::cout << "Not enough parameters" << std::endl;
        return 1;
    }
    else if (argc > 4)
    {
        std::cout << "Too many parameters" << std::endl;
        return 1;
    }
    else
    {
        std::string inputFileName;
        std::string outputFileName;
        double targetValue;

        inputFileName = args[1];
        outputFileName = argc == 4 ? args[3] : "out.csv";
        targetValue = std::stod(args[2]);

        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path intputFilePath = currentPath;
        std::filesystem::path outputFilePath = currentPath;

        intputFilePath /= inputFileName;
        outputFilePath /= outputFileName;

        std::ifstream ifs(intputFilePath);
        std::ofstream ofs(outputFilePath);

        if (!ifs.is_open())
        {
            std::cout << "Cannot open " << intputFilePath << std::endl;
            return 1;
        }

        if (!ofs.is_open())
        {
            std::cout << "Cannot open " << outputFilePath << std::endl;
            return 1;
        }

        std::vector<Employee> employees;

        std::string header;
        std::getline(ifs, header);

        ifs >> employees;

        int percentage = calculatePercentage(employees, targetValue);

        std::cout << "Percentage: " << percentage << std::endl;

        updateEmployees(employees, percentage);

        double actualValue = calculateValue(employees);

        std::cout << "Target Value: " << targetValue << std::endl;
        std::cout << "Actual Value: " << actualValue << std::endl;

        double adjustedValue = adjustRequiredVacationDays(employees, targetValue, percentage);

        std::cout << "Adjusted Value: " << adjustedValue << std::endl;

        header.erase(header.end() - 1);
        header += ",Required Days,Adjusted Days,Adjustment Difference in Days\n";

        ofs << header;
        ofs << employees;
    }

    return 0;
}