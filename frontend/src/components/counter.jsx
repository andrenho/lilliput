import React, { Component } from 'react';

class Counter extends Component {
    state = {
        count: 0,
    };

    render() { 
        let classes = "badge m-2 ";
        classes += (this.state.count === 0) ? "badge-warning" : "badge-primary";

        return (
            <React.Fragment>
                <span style={{ fontSize: 20 }} className={classes}>{this.state.count}</span>
                <button className="btn btn-secondary btn-sm">Increment</button>
            </React.Fragment>
        );
    }
}
 
export default Counter;