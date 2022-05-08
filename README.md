## fakehwclock: hwclock(8) for RTC-less systems.

### What is this for?
This is fake hwclock(8) tool which emulates behavior of original hwclock, but just
saves to or reads from a static file current datetime stamp. By default, it uses
`/etc/fakehwclock.sav` file, which is small binary file that contains dump of
`struct timespec` plus some identifying header and padding for future use.

To tell simply: this tool shall not be used on "normal" systems with /dev/rtc0
available (i.e. which contains typically battery powered Real Time Clock).
Those which don't (e.g. vanilla Raspberry PI without addons) could use this tool
just so their bootup time is not stuck in somewhere deep Unix past (01-01-1970).

### How to install?
Just type `make`, you'll need at least `gcc` for this. Code is simple. It shall
not encounter any troubles building it (unless it'll become very outdated in future).

Typing `make install` will move your distro shipped `/sbin/hwclock` to a `/sbin/hwclock.orig`
backup, so you'll not loose it if you in future will need it again. The new binary
will be placed on it's place.

### Running first time
Untill now on don't touch it, typically your distro init scripts (or whatever) will
call it with standard arguments on reboot/shutdown and only then `/etc/fakehwclock.sav`
will appear. On next boot, init scripts will call it again to obtain value from
that `/etc/fakehwclock.sav` file which was saved previously, and expected to set
clocks at least point of last shutdown time. Use `ntpd` to correct clocks after that.
That's it.

The process is automated (provided that your init scripts will call it with correct
arguments) and confirmed working for me in Slackware on an VisionFive V1 RISC-V board.

### UNSUPPORTED
systemd. I dunno how it manages this problem on RTC-less systems, it's huge clusterfuck.
I don't want to poke into it if it works or not. You can try and report (funny) results.

Time drifts and other timekeeping/NTP trickery is not supported at the moment.
(and probably never will)

### Compatibility
Most (if not all) command line arguments of standard hwclock(8) are accepted, but many
are simply ignored. The only operations supported by now are:

* `-r`: read `/etc/fakehwclock.sav` and report timestamp it contains in hwclock(8) format,
* `-w`: saving system time to `/etc/fakehwclock.sav`,
* `-s`: loading last saved timestamp from `/etc/fakehwclock.sav`,
* `-u`: giving program a hint it shall save as UTC,
* `-l`: giving program a hint it shall save as localtime.

Long options such as `--show` for `-r` are also supported.

`/etc/fakehwclock.sav` is binary dump of `struct timespec` used by kernel in it's
`clock_gettime(2)` and `clock_settime(2)` calls. This file shall not be considered portable.
