// Copyright 2025 Cameron Gagne

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <iomanip>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using boost::posix_time::time_from_string;
using boost::gregorian::date;

struct BootEvent {
    int startLineNo;
    int endLineNo;
    ptime startTime;
    bool completed = false;
    ptime endTime;
};

// Function to format date to "YYYY-MM-DD HH:MM:SS" format with numeric month
std::string format_datetime(const ptime& t) {
    std::ostringstream ss;
    ss.imbue(std::locale(ss.getloc(), new boost::posix_time::time_facet("%Y-%m-%d %H:%M:%S")));
    ss << t;
    return ss.str();
}

int main(int argc, char* argv[]) {
    std::string inName  = argv[1];
    std::string outName = inName + ".rpt";

    std::ifstream inFile(inName);
    std::ofstream outFile(outName);

    // regex date & time
    std::regex  r_dt(R"(^(\d{4}-\d{2}-\d{2})\s+(\d{2}:\d{2}:\d{2}(?:\.\d+)?))");

    // regex log.c.
    std::regex  r_start(R"(\(log\.c\.166\)\s*server\s+started)");

    // regex Boot-complete
    std::regex  r_complete(R"(oejs\.AbstractConnector:Started\s+SelectChannelConnector@(\d{1,3}(?:\.\d{1,3}){3}):(\d+))");

    std::string line;
    std::smatch m_dt, m_ev;

    std::vector<BootEvent> report;
    BootEvent pending;
    bool hasPending = false;

    int lineNo = 0;
    while (std::getline(inFile, line)) {
        ++lineNo;

        if (!std::regex_search(line, m_dt, r_dt))
            continue;

        std::string date_str = m_dt[1].str();
        std::string time_str = m_dt[2].str();
        ptime ts = time_from_string(date_str + " " + time_str);

        // check for a boot-start
        if (std::regex_search(line, m_ev, r_start)) {
            if (hasPending) {
                pending.completed = false;
                report.push_back(pending);
            }
            pending.startLineNo = lineNo;
            pending.startTime    = ts;
            pending.completed    = false;
            hasPending           = true;
        } else if (hasPending &&
            std::regex_search(line, m_ev, r_complete)) {   // check boot complete
            pending.endLineNo = lineNo;
            pending.completed = true;
            pending.endTime   = ts;
            report.push_back(pending);
            hasPending = false;
        }
    }

    // end of file
    if (hasPending) {
        pending.completed = false;
        report.push_back(pending);
    }

    // make output rpt
    for (auto& ev : report) {
        outFile << "=== Device boot ===" << std::endl;
        outFile << ev.startLineNo << "(" << inName << "): "
            << format_datetime(ev.startTime) << " Boot Start" << std::endl;

        if (ev.completed) {
            outFile << ev.endLineNo << "(" << inName << "): "
            << format_datetime(ev.endTime) << " Boot Completed" << std::endl;
            time_duration diff = ev.endTime - ev.startTime;
            int ms = diff.total_seconds() * 1000;
            outFile << "\tBoot Time: " << ms << "ms" << std::endl;
        } else {
            outFile << "**** Incomplete boot ****" << std::endl;
        }
        outFile << std::endl;
    }

    std::cout << "Report Written" << std::endl;
    return 0;
}
