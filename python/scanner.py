from bluepy.btle import Scanner, DefaultDelegate
import time

dir = "/home/pi/data/utility/pilogs/"

def writeData(kw):
    fname = time.strftime("%Y-%m-%d.dat")
    fo = open(dir + "/" + fname,"a")
    fo.write(" %s; %s\n" % (time.time(),kw) )
    fo.close()

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
        
    def handleDiscovery(self, dev, isNewDev, isNewData):
        if(dev.addr == "f7:61:e7:01:a3:e2"):
            for (adtype, desc, value) in dev.getScanData():
                if(desc == "Manufacturer"):
                    writeData(int(value[-4:],16))


            
scanner = Scanner().withDelegate(ScanDelegate())
scanner.start()
while(1):
    scanner.process()
scanner.stop()


