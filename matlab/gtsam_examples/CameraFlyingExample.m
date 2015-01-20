%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% GTSAM Copyright 2010, Georgia Tech Research Corporation, 
% Atlanta, Georgia 30332-0415
% All Rights Reserved
% Authors: Frank Dellaert, et al. (see THANKS for the full author list)
% 
% See LICENSE for the license information
%
% @brief A camera flying example through a field of cylinder landmarks
% @author Zhaoyang Lv
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


clear all;
clc;
clf;

import gtsam.*

% test or run
options.enableTests = false;

% the number of cylinders in the field
options.cylinder.cylinderNum = 15; % pls be smaller than 20
% cylinder size
options.cylinder.radius = 3;  % pls be smaller than 5
options.cylinder.height = 10;
% point density on cylinder
options.cylinder.pointDensity = 0.05;

%% set up the camera
%   parameters set according to the stereo camera:
%   http://www.matrix-vision.com/USB2.0-single-board-camera-mvbluefox-mlc.html

% set up monocular camera or stereo camera
options.camera.IS_MONO = false;
% the field of view of camera
options.camera.fov = 120;
% fps for image
options.camera.fps = 10;
% camera pixel resolution
options.camera.resolution = Point2(752, 480);
% camera horizon
options.camera.horizon = 60;
% camera baseline
options.camera.baseline = 0.05;
% camera focal length
options.camera.f = round(options.camera.resolution.x * options.camera.horizon / ...
    options.camera.fov);
% camera focal baseline
options.camera.fB = options.camera.f * options.camera.baseline;
% camera disparity
options.camera.disparity = options.camera.fB / options.camera.horizon;
% Monocular Camera Calibration
options.camera.monoK = Cal3_S2(options.camera.f, options.camera.f, 0, ...
    options.camera.resolution.x/2, options.camera.resolution.y/2);
% Stereo Camera Calibration
options.camera.stereoK = Cal3_S2Stereo(options.camera.f, options.camera.f, 0, ...
    options.camera.resolution.x/2, options.camera.resolution.y/2, options.camera.disparity);

% write video output
options.writeVideo = true;
% the testing field size (unit: meter)
options.fieldSize = Point2([100, 100]');
% camera flying speed (unit: meter / second)
options.speed = 20;
% number of camera poses in the simulated trajectory
options.poseNum = options.fieldSize.x / (options.speed / options.camera.fps);
% display covariance scaling factor
options.scale = 1;


%% This is for tests
if options.enableTests
    % test1: visibility test in monocular camera 
    cylinders{1}.centroid = Point3(30, 50, 5);
    cylinders{2}.centroid = Point3(50, 50, 5);
    cylinders{3}.centroid = Point3(70, 50, 5);

    for i = 1:3
        cylinders{i}.radius = 5;
        cylinders{i}.height = 10;

        cylinders{i}.Points{1} = cylinders{i}.centroid.compose(Point3(-cylinders{i}.radius, 0, 0));
        cylinders{i}.Points{2} = cylinders{i}.centroid.compose(Point3(cylinders{i}.radius, 0, 0));
    end

    camera = SimpleCamera.Lookat(Point3(10, 50, 10), ... 
        Point3(options.fieldSize.x/2, options.fieldSize.y/2, 0), ...
        Point3([0,0,1]'), options.monoK); 

    pose = camera.pose;
    prjMonoResult = cylinderSampleProjection(options.camera.monoK, pose, ...
        options.camera.resolution, cylinders);

    % test2: visibility test in stereo camera  
    prjStereoResult = cylinderSampleProjectionStereo(options.camera.stereoK, ...
        pose, options.camera.resolution, cylinders);
end

%% generate a set of cylinders and point samples on cylinders
cylinderNum = options.cylinder.cylinderNum;
cylinders = cell(cylinderNum, 1);
baseCentroid = cell(cylinderNum, 1);
theta = 0;
i = 1;
while i <= cylinderNum
    theta = theta + 2*pi/10;
    x = 40 * rand * cos(theta) + options.fieldSize.x/2;
    y = 20 * rand * sin(theta) + options.fieldSize.y/2;
    baseCentroid{i} = Point2([x, y]');
    
    % prevent two cylinders interact with each other
    regenerate = false;
    for j = 1:i-1
        if i > 1 && baseCentroid{i}.dist(baseCentroid{j}) < options.cylinder.radius * 2
            regenerate = true;
            break;
        end
    end
    if regenerate 
       continue;
    end
    
    cylinders{i,1} = cylinderSampling(baseCentroid{i}, options.cylinder.radius, ...
         options.cylinder.height, options.cylinder.pointDensity);
    i = i+1;
end

%% generate ground truth camera trajectories: a line
KMono = Cal3_S2(525,525,0,320,240);
cameraPoses = cell(options.poseNum, 1);
theta = 0;
for i = 1:options.poseNum    
    t = Point3([(i-1)*(options.fieldSize.x - 20)/options.poseNum + 20, ...
        15, 10]');
    camera = SimpleCamera.Lookat(t, ... 
        Point3(options.fieldSize.x/2, options.fieldSize.y/2, 0), ...
        Point3([0,0,1]'), options.camera.monoK);    
    cameraPoses{i} = camera.pose;
end


%% set up camera and get measurements
if options.camera.IS_MONO 
    % use Monocular Camera
    pts2dTracksMono = points2DTrackMonocular(options.camera.monoK, cameraPoses, ...
        options.camera.resolution, cylinders);
else 
    % use Stereo Camera
    [pts2dTracksStereo, estimateValuesStereo] = points2DTrackStereo(options.camera.stereoK, ...
        cameraPoses, options, cylinders);
end




 


