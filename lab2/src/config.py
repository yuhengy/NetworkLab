

TEST = 5


if TEST == 4:
  config = {
    'trial' : 2,
    'delay_all' : ['10ms', '100ms'],
    'block_all' : ['1M', '10M'],
    'bw_all' : [10, 50],
    'cpu' : -1,
    'queueSize' : -1
  }
elif TEST == 2:
  config = {
    'trial' : 5,
    'delay_all' : ['10ms', '50ms', '100ms', '500ms'],
    'block_all' : ['1M', '10M', '100M'],
    'bw_all' : [10, 50, 100, 500, 1000],
    'cpu' : -1,
    'queueSize' : -1
  }
elif TEST == 3:
  config = {
    'trial' : 1,
    'delay_all' : ['50ms'],
    'block_all' : ['1M', '10M'],
    'bw_all' : [1, 2, 5, 10, 20, 50, 100, 500, 1000],
    'cpu' : -1,
    'queueSize' : -1
  }
elif TEST == 4:
  config = {
    'trial' : 5,
    'delay_all' : ['10ms', '50ms', '100ms', '500ms'],
    'block_all' : ['1M', '10M', '100M'],
    'bw_all' : [1, 2, 5, 10, 20, 50, 100, 500, 1000],
    'cpu' : -1,
    'queueSize' : -1
  }
elif TEST == 5:
  config = {
    'trial' : 5,
    'delay_all' : ['10ms', '50ms', '100ms', '500ms'],
    'block_all' : ['1M', '10M', '100M'],
    'bw_all' : [10, 20, 50, 100, 500, 1000],
    'cpu' : 1.0,
    'queueSize' : 10000
  }

summayResultFile = './src/summayResult.json'
skipExistedLog = False
jobs = 1