var sock = require('./build/Release/binding.node');

var eth0 = sock.ifreq("eth0");
var lo   = sock.ifreq("lo");

var fd = sock.socket();

//-- eth0 --//

console.log( sock.ioctl(fd, sock.flags.SIOCGIFADDR, eth0) );
console.log( sock.decode(eth0, 'addr') );

console.log( sock.ioctl(fd, sock.flags.SIOCGIFNETMASK, eth0) );
console.log( sock.decode(eth0, 'addr') );

console.log( sock.ioctl(fd, sock.flags.SIOCGIFBRDADDR, eth0) );
console.log( sock.decode(eth0, "addr") );

console.log( sock.ioctl(fd, sock.flags.SIOCGIFHWADDR, eth0) );
console.log( sock.decode(eth0, "hwaddr") );

//-- loopback --//

console.log( sock.ioctl(fd, sock.flags.SIOCGIFADDR, lo) );
console.log( sock.decode(lo, 'addr') );

console.log( sock.ioctl(fd, sock.flags.SIOCGIFNETMASK, lo) );
console.log( sock.decode(lo, 'addr') );

console.log( sock.ioctl(fd, sock.flags.SIOCGIFBRDADDR, lo) );
console.log( sock.decode(lo, "addr") );

console.log( sock.ioctl(fd, sock.flags.SIOCGIFHWADDR, lo) );
console.log( sock.decode(lo, "hwaddr") );

