import json

"""
data['HVRACKS'][0]
for x in data['HVRACKS']:
  print x
for x in data['HVRACKS']:
  for y,z in x.iteritems():
    print y,z
for x in data['DIFS']:
  for y,z in x.iteritems():
    print y,z
for x in data['ASUS']:
  for y,z in x.iteritems():
    print y,z
for x in data['CHAMBERS']:
  for y,z in x.iteritems():
    print y,z

"""

class stparser:
    """
    Main class to create XML configuration file
    It handle a list of hosts ands a dictionnary of application instances
    """
    def __init__(self,name):
        """
        Initialise the hosts and instances storage
        A name and id are reserved (to be stored/retreive in a DB)
        """
        self.name=name;
        son_data=open(name)
        self.data=json.load(son_data)

    def createSQL(self,basename):
        """
        Create mysql table creation command from JSON data
        """
        fout=open("%s.sql" % basename,"w")
        fout.write("USE %s;\n" % basename)

        if (self.data.get("HVRACKS")):
            srack="INSERT INTO HVRACK "
      
            for x in self.data['HVRACKS']:
                vnam=[]
                vals=[]
                for y,z in x.iteritems():
                    vnam.append(y)
                    vals.append('\"%s\"' % z)
                snam="("
                sval=" VALUES("
                for i in range(0,len(vnam)-1):
                    snam=snam+vnam[i]+","
                    sval=sval+str(vals[i])+","
                i=len(vnam)-1
                snam=snam+vnam[i]+")"
                sval=sval+str(vals[i])+");"
                fout.write(srack+snam+sval+"\n");


        if (self.data.get("DIFS")):                
            srack="INSERT INTO DIF "
            for x in self.data['DIFS']:
                vnam=[]
                vals=[]

                for y,z in x.iteritems():
                    vnam.append(y)
                    vals.append('\"%s\"' % z)
                snam="("
                sval=" VALUES("
                for i in range(0,len(vnam)-1):
                    snam=snam+vnam[i]+","
                    sval=sval+str(vals[i])+","
                i=len(vnam)-1
                snam=snam+vnam[i]+")"
                sval=sval+str(vals[i])+");"
                fout.write(srack+snam+sval+"\n");
        if (self.data.get("ASUS")):                
            srack="INSERT INTO ASU "
            for x in self.data['ASUS']:
                vnam=[]
                vals=[]

                for y,z in x.iteritems():
                    vnam.append(y)
                    vals.append('\"%s\"' % z)
                snam="("
                sval=" VALUES("
                for i in range(0,len(vnam)-1):
                    snam=snam+vnam[i]+","
                    sval=sval+str(vals[i])+","
                i=len(vnam)-1
                snam=snam+vnam[i]+")"
                sval=sval+str(vals[i])+");"
                fout.write(srack+snam+sval+"\n");
        if (self.data.get("CHAMBERS")):                
            srack="INSERT INTO CHAMBER "
            for x in self.data['CHAMBERS']:
                vnam=[]
                vals=[]
                for y,z in x.iteritems():
                    vnam.append(y)
                    vals.append('\"%s\"' % z)
                snam="("
                sval=" VALUES("
                for i in range(0,len(vnam)-1):
                    snam=snam+vnam[i]+","
                    sval=sval+str(vals[i])+","
                i=len(vnam)-1
                snam=snam+vnam[i]+")"
                sval=sval+str(vals[i])+");"
                fout.write(srack+snam+sval+"\n");

        fout.write("INSERT INTO SETUP (NAME,START) VALUES(\"%s\",NOW());\n" % self.data["SETUP"]["NAME"])
        srack="INSERT INTO DETECTOR "
        for x in self.data['SETUP']['DETECTORS']:
            vnam=[]
            vals=[]
            for y,z in x.iteritems():
                if y=="CHAMBERID":
                    dv='(SELECT IDX FROM CHAMBER WHERE NAME=\"%s\" ORDER BY IDX DESC LIMIT 1)' % z
                    vnam.append(y)
                    vals.append(dv)
                    continue
                if y=="HVRACKID":
                    dv='(SELECT IDX FROM HVRACK WHERE HOSTNAME=\"%s\" ORDER BY IDX DESC LIMIT 1 )' % z
                    vnam.append(y)
                    vals.append(dv)
                    continue
                if y=="DIFID0":
                    dv='(SELECT IDX FROM DIF WHERE NUMBER=%d ORDER BY IDX DESC LIMIT 1)' % z
                    vnam.append(y)
                    vals.append(dv)
                    continue
                if y=="DIFID1":
                    dv='(SELECT IDX FROM DIF WHERE NUMBER=%d ORDER BY IDX DESC LIMIT 1)' % z
                    vnam.append(y)
                    vals.append(dv)
                    continue
                if y=="DIFID2":
                    dv='(SELECT IDX FROM DIF WHERE NUMBER=%d ORDER BY IDX DESC LIMIT 1)' % z
                    vnam.append(y)
                    vals.append(dv)
                    continue
                if y=="ASUID00":
                    dv='(SELECT IDX FROM ASU WHERE NAME=\"%s\" ORDER BY IDX DESC LIMIT 1)' % z
                    vnam.append(y)
                    vals.append(dv)
                    continue
                
                vnam.append(y)
                vals.append('\"%s\"' % z)
            snam="(SETUPID,"
            sval=" VALUES((SELECT IDX FROM SETUP WHERE NAME=\"%s\" ORDER BY IDX DESC LIMIT 1)," % self.data["SETUP"]["NAME"] 
            for i in range(0,len(vnam)-1):
                snam=snam+vnam[i]+","
                sval=sval+str(vals[i])+","
            i=len(vnam)-1
            snam=snam+vnam[i]+")"
            sval=sval+str(vals[i])+");"
            fout.write(srack+snam+sval+"\n");



        fout.close()

    def createHeaders(self):
        """
        Create mysql table creation command from JSON data
        """
        for x in self.data['TABLES']:
            fout=open("%sDescription.h" %x,"w")
            fout.write("#ifndef _%sDescription_h\n" % x)
            fout.write("#define _%sDescription_h\n" % x)
            fout.write("#include <stdint.h>\n")
            fout.write("#include <string.h>\n")

            fout.write("class %sDescription \n { \n   private: \n" % x)
            ## private members
            y=self.data['TABLES'][x]
            ls=[]
            for st in y:
                for m,b in st.iteritems():
                    if b=="PRIMARY" or b== "INTEGER":
                        ls.append("int32_t %s_;" % m)
                        continue;
                    if b=="REAL":
                        ls.append("float %s_;" % m)
                        continue;
                    if b=="TIMESTAMP":
                        ls.append("char %s_[128];" %m);
                        continue;
                    if b=="BOOL":
                        #fout.write(m,"BOOLEAN DEFAULT 1")
                        ls.append("int32_t %s_;"% m)
                        continue;
                    if b[0:4]=="CHAR":
                        lenc=int(b[4:len(b)])
                        #fout.write(m,"VARCHAR(",lenc,")")
                        ls.append("char %s_[%d]; " % (m,lenc))
                        continue
                    ls.append("%s %s_;" % (m,b))
            lenst=len(ls)
            for i in range(0,lenst):
                fout.write("\t %s\n" % ls[i])
            #fout.write(ls[lenst-2])
            ## Constructor
            fout.write("\n   public: \n \t %sDescription(){;} \n" %x)
            ls=[]
            ls1=[]
            for st in y:
                for m,b in st.iteritems():
                    if b=="PRIMARY" or b== "INTEGER":
                        ls.append("int32_t %s" % m)
                        ls1.append("%s_=%s;" % (m,m))
                        continue;
                    if b=="REAL":
                        ls.append("float %s" % m)
                        ls1.append("%s_=%s;" % (m,m))
                        continue;
                    if b=="TIMESTAMP":
                        ls.append("char* %s" %m);
                        ls1.append("memcpy(%s_,%s,strlen(%s));" % (m,m,m))
                        continue;
                    if b=="BOOL":
                        #fout.write(m,"BOOLEAN DEFAULT 1")
                        ls.append("int32_t %s"% m)
                        ls1.append("%s_=%s;" % (m,m))
                        continue;
                    if b[0:4]=="CHAR":
                        lenc=int(b[4:len(b)])
                        #fout.write(m,"VARCHAR(",lenc,")")
                        ls.append("char* %s" % (m))
                        ls1.append("memcpy(%s_,%s,strlen(%s));" % (m,m,m))
                        continue
                    ls1.append("%s_=%s;" % (m,m))
            lenst=len(ls)

            sarg=""
            sexec=""
            for i in range(0,lenst-1):
                sarg=sarg+ls[i]+","
                sexec=sexec+ls1[i]
            sarg=sarg+ls[lenst-1]
            sexec=sexec+ls1[lenst-1]
            fout.write("\t %sDescription(%s){%s}\n" % (x,sarg,sexec))

            ### Getter
            ls=[]
            ls1=[]
            for st in y:
                for m,b in st.iteritems():
                    if b=="PRIMARY" or b== "INTEGER":
                        ls.append("\t inline int32_t get%s(){return %s_;}" % (m,m))
                        ls1.append("\t inline void set%s(int32_t s){%s_=s;}" % (m,m))
                        continue;
                    if b=="REAL":
                        ls.append("\t inline float get%s(){return %s_;}" % (m,m))
                        ls1.append("\t inline void set%s(float s){%s_=s;}" % (m,m))
                        continue;
                    if b=="TIMESTAMP":
                        ls.append("\t  inline char* get%s(){return %s_;}" % (m,m))
                        ls1.append("\t  inline void set%s(char* s){memcpy(%s_,s,strlen(s));}" % (m,m))
                        continue;
                    if b=="BOOL":
                        #fout.write(m,"BOOLEAN DEFAULT 1")
                        ls.append("\t  inline int32_t get%s(){return %s_;}" % (m,m))
                        ls1.append("\t  inline void set%s(int32_t s){%s_=s;}" % (m,m))
                        continue;
                    if b[0:4]=="CHAR":
                        ls.append("\t  inline char* get%s(){return %s_;}" % (m,m))
                        ls1.append("\t  inline void set%s(char* s){memcpy(%s_,s,strlen(s));}" % (m,m))
                        continue
            lenst=len(ls)

            for i in range(0,lenst):
                fout.write("%s \n" % ls[i])
            for i in range(0,lenst):
                fout.write("%s \n" % ls1[i])
            fout.write("};\n")
            fout.write("#endif\n")
            fout.close()
    def createMyProxy(self):
        """
        Create mysql table creation command from JSON data
        """
        for x in self.data['TABLES']:
            fout=open("%sMyProxy.h" %x,"w")
            fout.write("#ifndef _%sMyProxy_h\n" % x)
            fout.write("#define _%sMyProxy_h\n" % x)
            fout.write("#include \"%sDescription.h\"\n" % x)
            fout.write("#include \"MyInterface.h\"\n")
            fout.write("#include <stdint.h>\n")
            fout.write("#include <stdlib.h>\n")
            fout.write("#include <string>\n")
            fout.write("#include <iostream>\n")
            fout.write("#include <sstream>\n")
            fout.write("#include <map>\n")
            fout.write("class %sMyProxy \n { \n   private: \n" % x)
            ## Members
            fout.write("\t %sDescription* theDescription_;\n" % x)
            fout.write("\t std::map<uint32_t,%sDescription> theMap_;\n" % x)
            fout.write("\t uint32_t theIdx_;\n")
            fout.write("\t std::string theAccount_;\n")
            fout.write("\t MyInterface* my_;\n")
            fout.write("public:\n")
            fout.write("\t %sMyProxy(std::string acc);\n" %x)
            fout.write("\t void select(std::string cut=\"\");")
            fout.write("\t std::map<uint32_t,%sDescription>& getMap();\n" %x)
            fout.write("\t %sDescription& getDescription(uint32_t idx);\n" %x)
            fout.write("\t void insert(); \n")
            fout.write("};\n")
            fout.write("#endif\n")
            fout.close()

            fout=open("%sMyProxy.cc" %x,"w")

            fout.write("#include \"%sMyProxy.h\"\n" % x)
            ## Constructor
            s0="%sMyProxy::%sMyProxy(std::string acc) \n" % (x,x)
            s0+="{theAccount_=acc;\n"
            s0+="theIdx_=0;theMap_.clear();\n"
            s0+="theDescription_ = new %sDescription();\n" %x
            s0+="my_=new MyInterface(acc);\n}\n"
            fout.write(s0) 

            ## select
            y=self.data['TABLES'][x]
            ls=[]
            lst=[]
            for st in y:
                for m,b in st.iteritems():
                    ls.append(m)
                    lst.append(b)
            se="void %sMyProxy::select(std::string cut){\n" % x
            select="\"SELECT "
            lenst=len(ls)

            for i in range(0,lenst-1):
                select=select+ls[i]+","
            
            select=select+ls[lenst-1]+" from %s \"" %x
            
            se=se+" if (cut.length()==0) \n my_->executeSelect(%s);\n else \n" % select
            se=se+"{ std::stringstream ss(%s);ss<<\" WHERE \"<<cut; my_->executeSelect(ss.str());}\n" % select
            ## FILL THE MAP
            sf="theMap_.clear();\n"
            sf="MYSQL_ROW row=NULL;\n"
            sf=sf+"while ((row=my_->getNextRow())!=0) \n { \n"
            sf=sf+"%sDescription d; uint32_t idx;\n" % x
            for i in range(0,lenst):
                if (lst[i]=="PRIMARY"):
                    sf=sf+"d.set%s(atoi(row[%d]));\n" % (ls[i],i)
                    sf=sf+"idx=atoi(row[%d]);\n" % i
                if (lst[i]=="INTEGER"):
                    sf=sf+"d.set%s(atoi(row[%d]));\n" % (ls[i],i)
                if (lst[i]=="BOOL"):
                    sf=sf+"d.set%s(atoi(row[%d]));\n" % (ls[i],i)
                if (lst[i]=="REAL"):
                    sf=sf+"d.set%s(atof(row[%d]));\n" % (ls[i],i)
                if (lst[i]=="TIMESTAMP"):
                    sf=sf+"d.set%s(row[%d]);\n" % (ls[i],i)
                if (lst[i][0:4]=="CHAR"):
                     sf=sf+"d.set%s(row[%d]);\n" % (ls[i],i)
            sf=sf+"std::pair<uint32_t,%sDescription> p(idx,d);\n" %x
            sf=sf+"theMap_.insert(p);\n" 
            sf=sf+"}\n"
            se=se+sf
            se+="}\n"
            fout.write(se)
            ## Map access
            se="std::map<uint32_t,%sDescription>& %sMyProxy::getMap(){ return theMap_;}\n" % (x,x)
            fout.write(se)
            se="%sDescription& %sMyProxy::getDescription(uint32_t idx){ return theMap_[idx];}\n" % (x,x)
            fout.write(se)
            ## Update
            se="void %sMyProxy::insert(){ \n" %x
            se=se+"std::stringstream stmt;stmt.str(std::string());\n"
            se=se+"stmt<<\"INSERT INTO %s(" %x
            for i in range(0,lenst-1):
                if (lst[i]=="PRIMARY"):
                    continue
                se=se+ls[i]+","
            se=se+ls[lenst-1]
            se=se+") VALUES(\""
            for i in range(0,lenst-1):
                if (lst[i]=="PRIMARY"):
                    continue
                se=se+"<<theDescription_->get%s()<<\",\"" % ls[i]
            se=se+"<<theDescription_->get%s()<<\")\";\n" % ls[lenst-1]
            se=se+"my_->executeQuery(stmt.str());\n"
            se=se+"}\n"
            fout.write(se)
            

           
            fout.close()
            
