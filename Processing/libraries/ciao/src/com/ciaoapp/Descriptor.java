package com.ciaoapp;

import java.util.HashMap;
import java.util.Map;

public class Descriptor {

	private int n;
	private Map<String, String> values;
	
	public Descriptor() {
		n = 1;
		values = new HashMap<String, String>();
		values.put("format", "Messenger");
	}
	
	public void addDisplay(String label) {
		addDisplay(label, label);
	}
	
	public void addDisplay(String label, String prefix) {
		values.put("label" + n, label);
		values.put("prefix" + n, prefix);
		updateN();
	}
	
	public void addButton(String label) {
		addButton(label, label);
	}
	
	public void addButton(String label, String send) {
		values.put("label" + n, label);
		values.put("send" + n, send);
		updateN();
	}
	
	private void updateN() {
		values.put("n", Integer.toString(n));
		n++;
	}
	
	public Map<String, String> getValues() {
		return values;
	}
	
}
