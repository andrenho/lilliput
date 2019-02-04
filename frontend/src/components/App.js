import React, { Component } from 'react';
import Developer from './Developer';

class App extends Component {
  state = {
    selected: 0,
    links: [
      { text: "Virtual Machine", component: <div /> },
      { text: "Cartriges & disks", component: <div /> },
      { text: "Developer area", component: <Developer /> },
      { text: "Documentation", component: <div /> }
    ]
  };

  itemClass(i) {
    const cl = "nav-item";
    if (i === this.state.selected) {
      return cl + " active";
    }
    return cl;
  }

  changeSelected(i) {
    this.setState({
      selected: i,
    })
  }

  render() {
    return (
      <div>
        <nav className="navbar navbar-dark bg-dark navbar-expand-sm">
          <a className="navbar-brand" href="#!">
            luisavm
          </a>
          <button
            className="navbar-toggler"
            type="button"
            data-toggle="collapse"
            data-target="#navbar1"
          >
            <span className="navbar-toggler-icon" />
          </button>
          <div className="collapse navbar-collapse" id="navbar1">
            <ul className="navbar-nav mr-auto">
              {// TODO - active
              this.state.links.map((link, i) => {
                return (
                  <li key={link.text} className={this.itemClass(i)}>
                    <a
                      onClick={() => this.changeSelected(i)}
                      className="nav-link"
                      href="#!"
                    >
                      {link.text}
                    </a>
                  </li>
                );
              })}
            </ul>
          </div>
        </nav>
        { this.state.links[this.state.selected].component }
      </div>
    );
  }
}

export default App;
