#include <iostream>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <sys/mount.h>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

bool createMountPoint(const std::string& target) {
    struct stat st;
    if (stat(target.c_str(), &st) != 0) {
        if (mkdir(target.c_str(), 0755) != 0) {
            std::cerr << "Error creating mount point " << target << ": " << strerror(errno) << std::endl;
            return false;
        }
        std::cout << "Created mount point: " << target << std::endl;
    }
    return true;
}

bool mountUSBPartition(const std::string& source, const std::string& target) {
    if (!createMountPoint(target)) {
        return false;
    }
    std::string command = "sudo mount " + source + " " + target;
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Error mounting " << source << " to " << target << std::endl;
        return false;
    }
    std::cout << source << " mounted to " << target << std::endl;
    return true;
}

std::vector<std::string> getMountablePartitions(){
	std::vector<std::string> partitions;
	std::string command = "lsblk -ln -o NAME,TYPE,MOUNTPOINT | grep 'part' | awk '$3 == \"\" {print $1}'";
	std::string result = "";

	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe)
		throw std::runtime_error("popen() failed!");
	try{
		char buffer[128];
		while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
			result += buffer;
		}
	} catch (...) {
		pclose(pipe);
		throw;
	}
	pclose(pipe);

	std::istringstream ss(result);
	std::string line;

	while (std::getline(ss, line)) {
		if (!line.empty()) {
			partitions.push_back(line);
		}
	}
	return partitions;
}




int main() {
	std::vector<std::string> partitions = getMountablePartitions();
	std::cout<<"mountable partitions: " << std::endl;
	for (const auto& partition : partitions) {
		std::cout<<partition<<std::endl;
		std::string source = "/dev/" + partition; 
		std::string target = "/mnt/" + partition;
		if (mountUSBPartition(source, target)){
			std::cout<<"partition mounted :" << partition << std::endl;
		} else {
			std::cout<<"failed to mount."<<std::endl;
		}
	}
	

	
    	return 0;
}
