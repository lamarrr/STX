

// TODO(lamarrr): trim this down
enum class TaskEventType : uint8_t {
  Scheduled,
  Submitted,
  Executing,
  SuspendRequested,
  ForceSuspendRequested,
  ResumeRequested,
  ForceResumeRequested,
  Suspended,
  Resumed,
  Completed,
  CancelRequested,
  Canceled,
  ForceCanceled
};

enum class Cpu : uint32_t {};

struct CpuEvent {
  nanoseconds time_stamp{};
  Cpu cpu{0};
  TaskTraceInfo trace_info;
  TaskPriority priority = TaskPriority::Background;
};

struct TaskEvent {
  nanoseconds time_stamp{};
  TaskEventType type{TaskEventType::Scheduled};
  // as time since scheduler intialization
  TaskTraceInfo trace_info;
  TaskPriority priority = TaskPriority::Background;
};

// dump hook
struct SchedulerEvent {
  // time bubbles
  // suspend, resume,
  // ...
  //
};