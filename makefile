obj2jgr: obj2jgr.cpp                                                  
	    g++ -o obj2jgr obj2jgr.cpp                                        
                                                                      
run:                                                                  
	    ./obj2jgr 0 0 3 < cube.obj > cube1.jgr                            
	    ./obj2jgr 45 45 3 < cube.obj > cube2.jgr                          
	    ./obj2jgr 0 90 130 < fox.obj > fox1.jgr                           
	    ./obj2jgr 25 150 120 < fox.obj > fox2.jgr                         
	    ./obj2jgr 75 60 60 < fox.obj > fox3.jgr                           
	    ./jgraph cube1.jgr | convert -density 300 - -quality 100 cube1.jpg
	    ./jgraph cube2.jgr | convert -density 300 - -quality 100 cube2.jpg
	    ./jgraph fox1.jgr | convert -density 300 - -quality 100 fox1.jpg  
	    ./jgraph fox2.jgr | convert -density 300 - -quality 100 fox2.jpg  
	    ./jgraph fox3.jgr | convert -density 300 - -quality 100 fox3.jpg  
                                                                      
clean:                                                                
	    rm obj2jgr                                                        
                                                                      
cleanj:                                                               
	    rm *.jgr *.jpg                                                    
