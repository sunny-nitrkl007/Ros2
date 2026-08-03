#!/usr/bin/env python3
import sys,time,rclpy
from rclpy.node import Node
from lps_interfaces.msg import PassTrackerDebug
from lps_interfaces.srv import JobManagerCommand
from std_srvs.srv import SetBool
class V(Node):
 def __init__(self):
  super().__init__('lps_pass_tracker_validation');self.m=None
  self.create_subscription(PassTrackerDebug,'/loader/job_manager/pass_tracker_debug',self.cb,50)
  self.cmd=self.create_client(JobManagerCommand,'/loader/job_manager/command');self.ctl=self.create_client(SetBool,'/loader/scenario/enabled')
 def cb(self,m):self.m=m
 def wait(self,p,t,name):
  end=time.monotonic()+t
  while rclpy.ok() and time.monotonic()<end:
   rclpy.spin_once(self,timeout_sec=.05)
   if self.m and p(self.m):self.get_logger().info('PASS: '+name);return True
  self.get_logger().error('FAIL: '+name);return False
 def call(self,c,r):
  if not c.wait_for_service(timeout_sec=3):raise RuntimeError('service unavailable')
  f=c.call_async(r);end=time.monotonic()+3
  while not f.done() and time.monotonic()<end:rclpy.spin_once(self,timeout_sec=.05)
  if not f.done():raise RuntimeError('service timeout')
  return f.result()
 def command(self,n,i):
  r=JobManagerCommand.Request();r.requester='m2_1_auto';r.legacy_request_id=i;r.command=n
  x=self.call(self.cmd,r)
  if not x.accepted:raise RuntimeError('command rejected')
 def enable(self,b):
  r=SetBool.Request();r.data=b;self.call(self.ctl,r)
 def run(self):
  if not self.wait(lambda m:m.add_pass and m.add_pass_accuracy==3 and abs(m.add_pass_weight-2.5)<.01,12,'normal add-pass event'):return False
  if not self.wait(lambda m:m.pass_count>=2 and not m.truck_pass_active,8,'two completed passes'):return False
  c,w=self.m.pass_count,self.m.truck_weight;self.command(2,201)
  if not self.wait(lambda m:m.remove_pass,2,'minus-one event'):return False
  if not self.wait(lambda m:m.pass_count==c-1 and abs(m.truck_weight-(w-2.5))<.01,2,'minus-one state'):return False
  self.command(3,202)
  if not self.wait(lambda m:m.clear,2,'clear event'):return False
  if not self.wait(lambda m:m.pass_count==0 and abs(m.truck_weight)<.01,2,'clear state'):return False
  if not self.wait(lambda m:m.pass_count>=1 and not m.truck_pass_active,8,'pass for store'):return False
  self.command(4,203)
  if not self.wait(lambda m:m.store,2,'store event'):return False
  if not self.wait(lambda m:m.pass_count==0 and abs(m.truck_weight)<.01,2,'store state'):return False
  self.enable(False)
  if not self.wait(lambda m:not m.input_fresh,3,'stale input detected'):return False
  c,w=self.m.pass_count,self.m.truck_weight;end=time.monotonic()+1;bad=False
  while time.monotonic()<end:
   rclpy.spin_once(self,timeout_sec=.05);bad|=bool(self.m.add_pass or self.m.remove_pass or self.m.clear or self.m.store)
  if bad or self.m.pass_count!=c or abs(self.m.truck_weight-w)>.01:self.get_logger().error('FAIL: stale state safety');return False
  self.get_logger().info('PASS: stale state retained without events');self.enable(True)
  if not self.wait(lambda m:m.input_fresh,3,'scenario recovery'):return False
  self.get_logger().info('ALL M2.1 AUTOMATED VALIDATIONS PASSED');return True
def main():
 rclpy.init();n=V()
 try:ok=n.run()
 except Exception as e:n.get_logger().error(str(e));ok=False
 n.destroy_node();rclpy.shutdown();return 0 if ok else 1
if __name__=='__main__':sys.exit(main())
