using System;
using System.Collections.Concurrent;
using System.Threading;
using UnityEngine;

namespace Cambrian.iOS
{
    public class QueueThread
    {
        private ConcurrentQueue<Action> _bgQueue = new ConcurrentQueue<Action>();
        private Thread _bgQueueThread;
        private bool _bgQueueThreadRunning;
        private readonly EventWaitHandle _waitHandle = new AutoResetEvent(false);

        
        public void Start()
        {
            Stop();
            _bgQueueThread = new Thread(ThreadLoop);
            _bgQueueThread.Start();
        }
        
        public void Stop()
        {
            _bgQueueThreadRunning = false;
            _bgQueue = new ConcurrentQueue<Action>();
        }
        
        public void Enqueue(Action action)
        {
            _bgQueue.Enqueue(action);
        }

        private void ThreadLoop()
        {      
            _bgQueueThreadRunning = true;
            while (_bgQueueThreadRunning)
            {
                Action action;
                
                if (_bgQueue.TryDequeue(out action))
                {
                    action();
                }
                
                Thread.Sleep(20);//todo, wait on condition triggered by enqueue
            }
        }
    }
}