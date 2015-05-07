package net;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Date;




public class MySQLAccess {
  private Connection connect = null;
  private Statement statement = null;
  private PreparedStatement preparedStatement = null;
  private ResultSet resultSet = null;

  
  public MySQLAccess(String jdbc, String usr, String pw){
   try {
	connect = DriverManager.getConnection(jdbc, usr, pw);
	} catch (SQLException e) {
		// TODO Auto-generated catch block
		e.printStackTrace();
	}
	  
  }
  
  public void queryDB(String query) throws Exception {
    try {
      statement = connect.createStatement();
      resultSet = statement.executeQuery(query);
      writeResultSet(resultSet);

      
    } catch (Exception e) {
      throw e;
    } finally {
      close();
    }

  }
  
  public String getStringFromqueryDB(String query,String var) throws Exception {
	  
	  
	    try {
	      statement = connect.createStatement();
	      resultSet = statement.executeQuery(query);
	      //writeResultSet(resultSet);
	      return getStringFromResultSet(resultSet, var);

	      
	    } catch (Exception e) {
	      throw e;
	    }/* finally {
	      close();
	    }*/

	  }
  
  public int getIntFromqueryDB(String query,String var) throws Exception {
	  
	  
	    try {
	      statement = connect.createStatement();
	      resultSet = statement.executeQuery(query);
	      //writeResultSet(resultSet);
	      return getIntFromResultSet(resultSet, var);

	      
	    } catch (Exception e) {
	      throw e;
	    }/* finally {
	      close();
	    }*/

	  }

  

  private void writeMetaData(ResultSet resultSet) throws SQLException {
    //   Now get some metadata from the database
    // Result set get the result of the SQL query
    
    System.out.println("The columns in the table are: ");
    
    System.out.println("Table: " + resultSet.getMetaData().getTableName(1));
    for  (int i = 1; i<= resultSet.getMetaData().getColumnCount(); i++){
      System.out.println("Column " +i  + " "+ resultSet.getMetaData().getColumnName(i));
    }
  }

  private void writeResultSet(ResultSet resultSet) throws SQLException {
    // ResultSet is initially before the first data set
    while (resultSet.next()) {
      // It is possible to get the columns via name
      // also possible to get the columns via the column number
      // which starts at 1
      // e.g. resultSet.getSTring(2);
      String user = resultSet.getString("username");
      String password = resultSet.getString("password");

      System.out.println("User: " + user);
      System.out.println("password: " + password);

    }
  }
  
  private String getStringFromResultSet(ResultSet resultSet, String var) throws SQLException {
	    // ResultSet is initially before the first data set
	  String ret_str = null; 
	    while (resultSet.next()) {
	      // It is possible to get the columns via name
	      // also possible to get the columns via the column number
	      // which starts at 1
	      // e.g. resultSet.getSTring(2);
	      
	      ret_str = resultSet.getString(var);
	    }
	    return  ret_str;
	  }
  
  private int getIntFromResultSet(ResultSet resultSet, String var) throws SQLException {
	    // ResultSet is initially before the first data set
	  int ret_int = 0; 
	    while (resultSet.next()) {
	      // It is possible to get the columns via name
	      // also possible to get the columns via the column number
	      // which starts at 1
	      // e.g. resultSet.getSTring(2);
	      
	      ret_int = resultSet.getInt(var);
	    }
	    return  ret_int;
	  }


  // You need to close the resultSet
  private void close() {
    try {
      if (resultSet != null) {
        resultSet.close();
      }

      if (statement != null) {
        statement.close();
      }

      if (connect != null) {
        connect.close();
      }
    } catch (Exception e) {

    }
  }

} 