
how to separate the .match()?


result move constructior is not correct

Result& operator=(Result&& rhs) { too

is_none move constructor

Template projects

Test operator==

examples: 

system/ulib/fs/include/fs/vfs.h OpenResult
system/ulib/intel-hda/utils/status.cc Status
system/ulib/intel-hda/include/intel-hda/utils/status_or.h StatusOr




Every change of state must be followed by a destruction (and construction if it has a non-null variant)

delete panic throw