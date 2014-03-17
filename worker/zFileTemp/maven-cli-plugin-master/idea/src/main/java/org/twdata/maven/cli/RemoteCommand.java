package org.twdata.maven.cli;

import com.intellij.openapi.util.JDOMExternalizable;
import com.intellij.openapi.util.InvalidDataException;
import com.intellij.openapi.util.DefaultJDOMExternalizer;
import com.intellij.openapi.util.WriteExternalException;

import java.io.Serializable;

import org.jdom.Element;

/**
 * Configuration used to call a remote CLI instance
 */
public class RemoteCommand {
    private String host;
    private int port;
    private String command;

    public RemoteCommand() {
    }

    public RemoteCommand(String host, int port, String command) {
        this.host = host;
        this.port = port;
        this.command = command;
    }

    public RemoteCommand(Element e) {
        this.host = e.getAttributeValue("host");
        this.port = Integer.parseInt(e.getAttributeValue("port"));
        this.command = e.getAttributeValue("command");
    }

    public String getHost() {
        return host;
    }

    public void setHost(String host) {
        this.host = host;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public String getCommand() {
        return command;
    }

    public void setCommand(String command) {
        this.command = command;
    }

    public Element toXml() {
        Element e = new Element("command");
        e.setAttribute("host", host);
        e.setAttribute("port", String.valueOf(port));
        e.setAttribute("command", command);
        return e;
    }
}
