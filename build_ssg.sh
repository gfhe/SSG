test_ssg_index  data/glove_50k_50.fvecs data/glove_50k_50.graph 100 50 60 data/glove.ssg



## ssg recommand
## get data: curl -O 128.199.246.54:566/glove_50k_400.graph
test_ssg_index  data/glove_50k_50.fvecs data/glove_50k_50.graph 500 50 60 data/glove.500.ssg
test_ssg_index  data/glove_50k_50.fvecs data/glove_50k_50.graph 500 50 60 data/glove.500.ssg.opt

