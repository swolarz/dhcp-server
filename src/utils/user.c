#include "user.h"

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>


const char* get_user_home() {
	struct passwd* pw = getpwuid(getuid());
	const char* homedir = pw->pw_dir;

	return homedir;
}
