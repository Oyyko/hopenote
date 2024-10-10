#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include <string>
#include <CoreServices/CoreServices.h>

class FileWatcher {
public:
  FileWatcher(const std::string &filename);
  ~FileWatcher();
  bool fileChanged();

private:
  static void callback(ConstFSEventStreamRef streamRef,
                       void *clientCallBackInfo, size_t numEvents,
                       void *eventPaths,
                       const FSEventStreamEventFlags eventFlags[],
                       const FSEventStreamEventId eventIds[]);

  FSEventStreamRef stream_;
  std::string filename_;
  bool changed_;
};

#endif // FILE_WATCHER_H
