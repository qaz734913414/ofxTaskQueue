//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#include "ofApp.h"


void ofApp::setup()
{
    ofEnableAlphaBlending();
    ofSetFrameRate(60);

    // Add capacity to the thread pool.
    // Poco::ThreadPool::defaultPool().addCapacity(100);

    // Limit the maximum number of tasks for shared thread pools.
    queue.setMaximumTasks(10);

    // Register to receive task queue events.
    queue.registerTaskProgressEvents(this);

    // Optionally listen for task custom notifications.
    ofAddListener(queue.onTaskCustomNotification,
                  this,
                  &ofApp::onTaskCustomNotification);

    for (int i = 0; i < 1000; ++i)
    {
        std::string name = "Counting Task #" + ofToString(i);

        // The task queue will take ownership of the task,
        // so tasks can be passed like this.
        queue.start(ofToString(i), new SimpleCountingTask(name, 100));
    }
}


void ofApp::update()
{
    TaskProgress::iterator iter = taskProgress.begin();

    while (iter != taskProgress.end())
    {
        // Get a reference to the task in our map.
        SimpleTaskProgress& task = iter->second;

        // If it is finished and done fading, then erase it from our map.
        if (task.state == Poco::Task::TASK_FINISHED && task.fader <= 0)
        {
            taskProgress.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
}


void ofApp::draw()
{
    ofBackground(0);

    std::stringstream ss;

    ss << "Press 'c' to cancel all tasks." << std::endl;
    ss << "Press 'a' to add tasks." << std::endl;

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(10, 14));

    ss.str("");
    ss << "Total Active Tasks: " << queue.getActiveCount() << std::endl;
    ss << "Total Queued Tasks: " << queue.getQueuedCount();

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(ofGetWidth() / 2, 14));

    int height = 20;
    int y = height * 3;

    TaskProgress::iterator iter = taskProgress.begin();
    
    while (iter != taskProgress.end())
    {
        // Get a reference to the task in our map.
        SimpleTaskProgress& task = iter->second;

        // Call its draw function.
        task.draw(0, y, ofGetWidth(), height);

        // Increment our height.
        y += (height + 5);

        // Increment our iterator.
        ++iter;
    }
}


void ofApp::exit()
{
    // It's a good practice to unregister the events.
    queue.unregisterTaskProgressEvents(this);

    // Unregister the optional notification listener.
    ofRemoveListener(queue.onTaskCustomNotification,
                     this,
                     &ofApp::onTaskCustomNotification);
}


void ofApp::keyPressed(int key)
{
    if ('c' == key)
    {
        queue.cancelAll();
    }
    else if ('a')
    {
        queue.start(ofToString(ofRandom(1)), new SimpleCountingTask("User manually added!", 100));
    }
}


void ofApp::onTaskQueued(const ofx::TaskQueueEventArgs& args)
{
    taskProgress[args.taskId()] = SimpleTaskProgress(args);
}


void ofApp::onTaskStarted(const ofx::TaskQueueEventArgs& args)
{
    taskProgress[args.taskId()].update(args);
}


void ofApp::onTaskCancelled(const ofx::TaskQueueEventArgs& args)
{
    taskProgress[args.taskId()].update(args);
}


void ofApp::onTaskFinished(const ofx::TaskQueueEventArgs& args)
{
    // This is always called last, even after a task failure.
    // We do not remove the task progress here because we want to
    // keep it around and display it.  We will remove it when it
    // expires during a future update loop.
    taskProgress[args.taskId()].update(args);
}


void ofApp::onTaskFailed(const ofx::TaskFailedEventArgs& args)
{
    taskProgress[args.taskId()].update(args);
}


void ofApp::onTaskProgress(const ofx::TaskProgressEventArgs& args)
{
    taskProgress[args.taskId()].update(args);
}


void ofApp::onTaskCustomNotification(const ofx::TaskCustomNotificationEventArgs& args)
{
    int i = 0;

    std::string message;

    if (args.extract(i))
    {
        ofLogVerbose("ofApp::onTaskCustomNotification") << "Parsed a custom notification task " << args.taskId() << " with int = " << i << std::endl;
    }
    else if (args.extract(message))
    {
        taskProgress[args.taskId()].data = message;
    }
    else
    {
        std::cout << "Got an unknown custom notification! Name: " << args.notification()->name() << std::endl;
    }
}
