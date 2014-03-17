package org.twdata.maven.cli.config;

import org.twdata.maven.cli.CliClientProjectComponent;
import org.twdata.maven.cli.RemoteCommand;

import javax.swing.*;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;

/**
 * Configuration form for commands
 */
public class CliClientConfigurationForm {
    private JTextField remoteHost1Field;
    private JLabel remoteHost1Label;
    private JLabel remotePort1Label;
    private JLabel remoteCommand1Label;
    private JTextField remoteCommand1Field;
    private JPanel command1Panel;
    private JPanel command2Panel;
    private JLabel remoteHost2Label;
    private JTextField remoteHost2Field;
    private JLabel remotePort2Label;
    private JLabel remoteCommand2Label;
    private JTextField remoteCommand2Field;
    private JPanel command3Panel;
    private JLabel remoteHost3Label;
    private JTextField remoteHost3Field;
    private JLabel remotePort3Label;
    private JLabel remoteCommand3Label;
    private JTextField remoteCommand3Field;
    private JTextField remotePort1Field;
    private JPanel remotePort1Panel;
    private JPanel remotePort2Panel;
    private JTextField remotePort2Field;
    private JPanel remotePort3Panel;
    private JTextField remotePort3Field;
    private JPanel rootComponent;

    public void setData(CliClientProjectComponent data) {
        remoteHost1Field.setText(data.getRemoteCommands()[0].getHost());
        remotePort1Field.setText(String.valueOf(data.getRemoteCommands()[0].getPort()));
        remoteCommand1Field.setText(data.getRemoteCommands()[0].getCommand());

        remoteHost2Field.setText(data.getRemoteCommands()[1].getHost());
        remotePort2Field.setText(String.valueOf(data.getRemoteCommands()[1].getPort()));
        remoteCommand2Field.setText(data.getRemoteCommands()[1].getCommand());

        remoteHost3Field.setText(data.getRemoteCommands()[2].getHost());
        remotePort3Field.setText(String.valueOf(data.getRemoteCommands()[2].getPort()));
        remoteCommand3Field.setText(data.getRemoteCommands()[2].getCommand());
    }

    public void getData(CliClientProjectComponent data) {
        data.getRemoteCommands()[0].setHost(remoteHost1Field.getText());
        data.getRemoteCommands()[0].setPort(Integer.parseInt(remotePort1Field.getText()));
        data.getRemoteCommands()[0].setCommand(remoteCommand1Field.getText());

        data.getRemoteCommands()[1].setHost(remoteHost2Field.getText());
        data.getRemoteCommands()[1].setPort(Integer.parseInt(remotePort2Field.getText()));
        data.getRemoteCommands()[1].setCommand(remoteCommand2Field.getText());

        data.getRemoteCommands()[2].setHost(remoteHost3Field.getText());
        data.getRemoteCommands()[2].setPort(Integer.parseInt(remotePort3Field.getText()));
        data.getRemoteCommands()[2].setCommand(remoteCommand3Field.getText());
    }

    public boolean isModified(CliClientProjectComponent data) {
        for (int pos = 1; pos <= 3; pos++) {
            for (String type : new String[]{"Host", "Port", "Command"}) {
                if (isFieldModified(data, type, pos)) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean isFieldModified(CliClientProjectComponent data, String type, int pos) {
        try {
            Field field = getClass().getDeclaredField("remote" + type + pos + "Field");
            field.setAccessible(true);
            String fieldValue = ((JTextField) field.get(this)).getText();
            RemoteCommand cmd = data.getRemoteCommands()[pos - 1];
            String dataValue = String.valueOf(cmd.getClass().getMethod("get" + type).invoke(cmd));
            return fieldValue != null ?
                    !fieldValue.equals(dataValue) :
                    dataValue != null;
        } catch (NoSuchFieldException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        }
    }

    public JPanel getRootComponent() {
        return rootComponent;
    }
}
