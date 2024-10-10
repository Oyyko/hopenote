#include "file_watcher.hpp"
#include <iostream>

FileWatcher::FileWatcher(const std::string &filename)
    : filename_(filename), changed_(false) {
  CFStringRef path =
      CFStringCreateWithCString(NULL, filename_.c_str(), kCFStringEncodingUTF8);
  CFArrayRef pathsToWatch = CFArrayCreate(nullptr, (const void **)&path, 1, nullptr);

  FSEventStreamContext context;
  context.version = 0;
  context.info = this;
  context.retain = nullptr;
  context.release = nullptr;
  context.copyDescription = nullptr;

  stream_ = FSEventStreamCreate(NULL, &FileWatcher::callback, &context,
                                pathsToWatch, kFSEventStreamEventIdSinceNow,
                                0.1, kFSEventStreamCreateFlagFileEvents);

  FSEventStreamScheduleWithRunLoop(stream_, CFRunLoopGetCurrent(),
                                   kCFRunLoopDefaultMode);
  FSEventStreamStart(stream_);

  CFRelease(pathsToWatch);
  CFRelease(path);
}

FileWatcher::~FileWatcher() {
  FSEventStreamStop(stream_);
  FSEventStreamInvalidate(stream_);
  FSEventStreamRelease(stream_);
}

bool FileWatcher::fileChanged() {
  bool changed = changed_;
  changed_ = false;
  return changed;
}

void FileWatcher::callback(ConstFSEventStreamRef streamRef,
                           void *clientCallBackInfo, size_t numEvents,
                           void *eventPaths,
                           const FSEventStreamEventFlags eventFlags[],
                           const FSEventStreamEventId eventIds[]) {
  FileWatcher *watcher = static_cast<FileWatcher *>(clientCallBackInfo);
  watcher->changed_ = true;
}