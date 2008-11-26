/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KROSSIMPL_H
#define KROSSIMPL_H

#include "duchainreader.h"

class KrossImpl : public DUChainReader
{
    public:
        KrossImpl(KDevelop::TopDUContext* top) : DUChainReader(top) {}
        QString output;
        QString handlersHeader;
        
        void writeDocument()
        {
            handlersHeader += "#ifndef "+filename.toUpper()+"_H\n";
            handlersHeader += "#define "+filename.toUpper()+"_H\n\n";
            handlersHeader += "#include<QtCore/QVariant>\n\n";
            handlersHeader += "//This is file has been generated by xmltokross, "
                              "you should not edit this file but the files used to generate it.\n\n";
                              
            output += "//This is file has been generated by xmltokross,"
                        "you should not edit this file but the files used to generate it.\n\n"
                      "#include <QtCore/QObject>\n"
                      "#include <QtCore/QVariant>\n"
                      "#include <kross/core/object.h>\n"
                      "#include <kross/core/manager.h>\n"
                      "#include <kross/core/wrapperinterface.h>\n";
                      "#include \""+filename+"\"\n";
            foreach(const QString & include, includes)
            {
                output += "#include <"+include+">\n";
//                 handlersHeader += "#include <"+include+">\n";
            }
            output +='\n';
        }
        
        QString toKrossName(const QString &name)
        {
            return "KrossImpl"+QString(name).replace("::", QString());
        }
        
        void writeClass(const QString& classname, const QString& baseClass, const QList<QStringList>& enums)
        {
            classNamespace[classname]=inNamespace;
            
            qDebug() << "writeClass: " << classNamespace[classname] << "::"
                     << classname << definedClasses << " : public " << baseClass;
            QString krossClassname=toKrossName(classname);
//             if(baseClass.isEmpty())
                output += "class " + krossClassname + " : public "+classname+'\n';
//             else
//                 output += "class " + krossClassname + " : public "+toKrossName(baseClass)+'\n';
            
            output += "{\n";
            
            foreach(const QStringList& en, enums)
            {
                writeQ_Enum(en);
            }
            
            output += "\tpublic:\n";
            
            foreach(const QStringList& en, enums)
            {
                writeEnum(en);
            }
            
//             output += "\t\tKross::Object::Ptr wrappedObject() const { return wrapped; }\n\n";
        }
        
        void writeEndClass()
        {
            output += "\tprivate:\n"
                      "\t\tKross::Object::Ptr obj;\n"
                      "};\n\n";
        }
                    
        void writeVariable(const QString& name, const QString& _type, bool isConst)
        {}
        
        void writeNamespace(const QString& name)
        {
            output += "using namespace "+name+";\n\n";
        }
        
        void writeEndEnum(const QStringList& ) { Q_ASSERT(false); }
        
        void writeEnum(const QStringList &fl)
        {}
        
        void writeQ_Enum(const QStringList& fl)
        {}
        
        void createHandler(const QString& _classname)
        {}
        
        void writeEndDocument()
        {}
        
        void writeEndFunction(const method& m)
        {
            if(!m.isVirtual && !m.isConstructor)
                return;
            
            QString classname=definedClasses.last();
            qDebug() << "writing down" << classname;
            if(m.isConstructor)
            {
                QString krossClassname=toKrossName(classname);
                output += "\t\t"+krossClassname+"(Kross::Object::Ptr _obj";
                if(!m.args.isEmpty())
                    output+=", ";
            }
            else
            {
                QString rettype=m.returnType;
                rettype=rettype.replace('&', QString());
                if(!rettype.contains('*'))
                    rettype=rettype.replace("const ", QString());
                output += "\t\t" + rettype +' '+ m.funcname+'(';
            }
            QStringList values;
            
            int param=0;
            foreach(const method::argument& arg, m.args)
            {
                QString varname=arg.name;
                if(varname.isEmpty()) {
                    qWarning() << "The paramenter number "+QString::number(param)+" in method: "+
                            inNamespace+"::"+definedClasses.last()+"::"+m.funcname+" does not have a name";
                    varname=QString("x%1").arg(param);
                }
                values += varname;
                output += arg.type +' '+ varname;
                if(!arg.def.isEmpty())
                    output+='='+arg.def;
                output += ", ";
                param++;
            }
            if(!values.isEmpty())
                output.resize(output.size()-2);
            output += ')';
            
            QString shouldReturn= m.returnType=="void" ? QString() : QString("return ");
            QString params=values.join(", ");
            
            if(m.isConstructor)
            {
                output += " : "+classname+"("+params+"), obj(_obj) {}\n";
            }
            else
            {
                if(m.isConst)
                    output+=" const";
                output += " {\n";
                
                if(!m.isAbstract) {
                    output+="\t\t\tif(!obj->methodNames().contains(\""+m.funcname+"\"))\n\t\t\t\t"+
                        shouldReturn+definedClasses.last()+"::"+m.funcname+"("+params+");\n"
                        "\t\t\telse\n\t";
                }
                QString variantParameters="QVariantList() <<" + values.join(" << ");
                QString call;
                if(values.isEmpty())
                    call="obj->callMethod(\""+m.funcname+"\")";
                else
                    call="obj->callMethod(\""+m.funcname+"\", "+variantParameters+')';
                output+="\t\t\t"+shouldReturn+call+".value<"+m.returnType+">();\n";
                output += "\t\t}\n\n";
            }
        }
};

#endif
