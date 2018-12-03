/*
 * test_tf.cpp
 *
 *  Created on: Jul 31, 2018
 *      Author: sujiwo
 */

#include <iostream>
#include <fstream>
#include <ros/package.h>
#include <boost/filesystem.hpp>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "utilities.h"
#include "datasets/MeidaiBagDataset.h"

using namespace std;
namespace bfs = boost::filesystem;


void dumpTrajectory(const string &dumpPath, const Trajectory &srcPath)
{
	ofstream fd(dumpPath);

	for (auto &tr: srcPath) {
		fd << dumpVector(tr.position()) << endl;
	}

	fd.close();
}


int main (int argc, char *argv[])
{
	auto meidaiDs = MeidaiBagDataset::load("/media/sujiwo/ssd/log_2016-12-26-13-21-10.bag");
	meidaiDs->setLidarParameters((getMyPath()/"params/64e-S2.yaml").string(), "", TTransform::Identity());
	auto lidarBag = meidaiDs->getLidarScanBag();

	lidarBag->setTimeConstraint(315.49, 932.16);

	Trajectory ndtTrack;
	createTrajectoryFromNDT2(*lidarBag,
		ndtTrack,
		meidaiDs->getGnssTrajectory(),
		"/home/sujiwo/Data/NagoyaUniversityMap/bin_meidai_ndmap.pcd");

	dumpTrajectory("/tmp/testndt.txt", ndtTrack);

	return 0;
}
