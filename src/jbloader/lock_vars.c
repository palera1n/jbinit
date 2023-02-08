#include <jbloader.h>

bool safemode_spin = true;
pthread_mutex_t safemode_mutex;

bool get_safemode_spin() {
    pthread_mutex_lock(&safemode_mutex);
    bool ret = safemode_spin;
    pthread_mutex_unlock(&safemode_mutex);
    return ret;
}

bool set_safemode_spin(bool val) {
    pthread_mutex_lock(&safemode_mutex);
    safemode_spin = val;
    pthread_mutex_unlock(&safemode_mutex);
    return val;
}
