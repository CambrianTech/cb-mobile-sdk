from kaffe.tensorflow import Network

class texture_nnet(Network):
    def setup(self):
        (self.feed('data')
             .conv(4, 4, 13, 3, 3, padding='VALID', relu=False, name='conv1')
             .max_pool(3, 3, 2, 2, padding='VALID', name='pool1')
             .relu(name='relu1')
             .conv(3, 3, 30, 1, 1, padding='VALID', relu=False, name='conv2')
             .avg_pool(3, 3, 2, 2, name='pool2')
             .relu(name='relu2')
             .fc(80, relu=False, name='ip1')
             .sigmoid(name='ip1_sig')
             .fc(5, relu=False, name='ip2')
             .fc(2, relu=False, name='feat'))

        (self.feed('data_p')
             .conv(4, 4, 13, 3, 3, padding='VALID', relu=False, name='conv1_p')
             .max_pool(3, 3, 2, 2, padding='VALID', name='pool1_p')
             .relu(name='relu1_p')
             .conv(3, 3, 30, 1, 1, padding='VALID', relu=False, name='conv2_p')
             .avg_pool(3, 3, 2, 2, name='pool2_p')
             .relu(name='relu2_p')
             .fc(80, relu=False, name='ip1_p')
             .sigmoid(name='ip1_sig_p')
             .fc(5, relu=False, name='ip2_p')
             .fc(2, relu=False, name='feat_p'))

        (self.feed('feat', 
                   'feat_p')
             .concat(3, name='concat')
             .fc(2, relu=False, name='fc_cls')
             .softmax(name='prob'))